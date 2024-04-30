#include "kvstore.h"
#include "config.h"
#include "sstable.h"
#include <cstdint>
#include <filesystem>
#include <string>
#include <chrono>
#include <vector>

KVStore::KVStore(const std::string &dir) : KVStoreAPI(dir)
{
	// Initialize the directory and timestamp
	this->SSTdir = dir;
	this->vLogdir = dir + "/vLog";
	this->sstMaxTimeStamp = 0;

	// Read all the sstables and write them into levelIndex
	this->sstFileCheck(this->SSTdir);

	// Initialize the memtable
	this->memtable = new MemTable();

	// Initialize the vlog
	this->vlog = new vLog(this->vLogdir);

	// Initialize the vlog offset
	this->curvLogOffset = 0;
}

KVStore::~KVStore()
{
	// Store and delete the memtable
	std::list<std::pair<uint64_t, std::string>> dataAll;
	dataAll = this->memtable->copyAll();

	// Write all the key-value pairs in memtable to sstable
	if(dataAll.size() > 0){
		// Get the current time to generate the filename
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		std::chrono::microseconds nstime = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
		
		// Update the timestamp
		this->sstMaxTimeStamp++;

		// Generate the filename
		std::string newFilePath = this->SSTdir + "/level-0/" + std::to_string(nstime.count()) + ".sst";
		SStable* newSSTable = new SStable(sstMaxTimeStamp, dataAll,  newFilePath, curvLogOffset);

		// Update the levelIndex
		this->levelIndex[0][nstime.count()] = newSSTable;

		// Reset the memtable
		this->memtable->reset();

		// Compact the sstable
		int checkResult = this->mergeCheck();
		while(checkResult != -1){
			this->merge(checkResult);
			checkResult = this->mergeCheck();
		}
	}

	// Delete the memtable
	delete this->memtable;

	// Delete all the sstables in memory
	for(auto level = this->levelIndex.begin(); level != this->levelIndex.end(); level++){
		for(auto sstable = level->second.begin(); sstable != level->second.end(); sstable++){
			delete sstable->second;
		}
	}

	// Delete the vlog
	delete this->vlog;
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{
	// Insert if the memtable is not full
	if(this->memtable->putCheck(key, s)){
		this->memtable->put(key, s);
		return;
	}
	
	// Write the key-value pair into sstable
	std::list<std::pair<uint64_t, std::string>> dataAll;
	dataAll = this->memtable->copyAll();

	// Get the current time to generate the filename
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	std::chrono::microseconds mstime = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());

	// Update the timestamp
	this->sstMaxTimeStamp++;

	// Generate the filename
	std::string newFilePath = this->SSTdir + "/level-0/" + std::to_string(mstime.count()) + ".sst";
	SStable* newSSTable = new SStable(sstMaxTimeStamp, dataAll,  newFilePath, curvLogOffset);

	// Update the levelIndex
	this->levelIndex[0][mstime.count()] = newSSTable;

	// Add the key-value pair into the vlog
	this->vlog->readFromList(dataAll);
	this->vlog->writeToFile(curvLogOffset);
	this->curvLogOffset = this->vlog->getHead();
	
	// Reset the memtable
	this->memtable->reset();

	// Compact the sstable
	int checkResult = this->mergeCheck();
	while(checkResult != -1){
		this->merge(checkResult);
		checkResult = this->mergeCheck();
	}

	// Insert the key-value pair into the memtable
	this->memtable->put(key, s);
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{	
	std::string res = this->memtable->get(key);

	// If the key is deleted, return empty string
	if(res == memtable_already_deleted)
		return "";

	if(res != memtable_not_exist)
		return res;

	// Check the sstables in levelIndex
	uint64_t latestTimeStamp = 0;
	for(auto level = this->levelIndex.begin(); level != this->levelIndex.end(); level++){
		bool isFound = false;

		// Tranverse all the sstables along the level
		for(auto sstable = level->second.rbegin(); sstable != level->second.rend(); sstable++){
			SStable *currentSSTable = sstable->second;
			// Check if the key is in the sstable
			if(currentSSTable->checkIfKeyExist(key)){
				uint64_t indexRes = currentSSTable->getKeyIndexByKey(key);
				// If not found, continue
				if(indexRes == UINT64_MAX)
					continue;

				// Get the value
				uint64_t targetOffset = currentSSTable->getSStableKeyOffset(indexRes);
				uint32_t targetLength = currentSSTable->getSStableKeyVlen(indexRes);
				std::string value = this->vlog->getValFromFile(this->vLogdir, targetOffset, targetLength);
				if(value == sstable_out_of_range)
					return "";
				if(currentSSTable->getSStableTimeStamp() >= latestTimeStamp){
					latestTimeStamp = currentSSTable->getSStableTimeStamp();
					res = value;
					isFound = true;
				}
			}
		}

		if(isFound && res == delete_tag)
			return "";
		else if(isFound)
			return res;
	}

	// Check the value is valid
	if(res == delete_tag || res == memtable_not_exist)
		return "";
	else
		return res;
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	if(this->get(key) == "")
		return false;
	this->put(key, delete_tag);
	return true;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
	// Delete the memtable
	utils::rmfile(logFilePath);
	this->memtable->reset();
	
	// Delete all the sstables in memory
	for(auto level = this->levelIndex.begin(); level != this->levelIndex.end(); level++){
		for(auto sstable = level->second.begin(); sstable != level->second.end(); sstable++){
			sstable->second->clear();
			delete sstable->second;
		}
	}
	this->levelIndex.clear();

	// Delete the vlog
	delete this->vlog;
}

/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 */
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list)
{
	// scanMap[key][timestamp] = value
	std::map<uint64_t, std::map<uint64_t, std::string> > scanMap;
	std::list<std::pair<uint64_t, std::string> >mergeList;

	// Scan the sstables in levelIndex
	for(auto level = this->levelIndex.begin(); level != this->levelIndex.end(); level++){
		for(auto sstable = level->second.begin(); sstable != level->second.end(); sstable++){
			SStable *currentSSTable = sstable->second;
			currentSSTable->scan(key1, key2, scanMap, *this->vlog);
		}
	}

	for(auto iter = scanMap.begin(); iter != scanMap.end(); iter++){
		uint64_t key = iter->first;
		for(auto iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++){
			list.push_back({key, iter2->second});
		}
	}

	// Scan the memtable
	this->memtable->scan(key1, key2, mergeList);

	// Merge the list
	for(auto iter = mergeList.begin(); iter != mergeList.end(); iter++){
		list.push_back({iter->first, iter->second});
	}
}

/**
 * Check the sstable in each level
 * Return the level that needs to be merged
 */
void KVStore::sstFileCheck(std::string dataPath){
	// Check if the directory exists
	if(!utils::dirExists(dataPath)){
		utils::mkdir(dataPath);
		return;
	}

	// Check all the sstables in the directory
	for(auto &p : std::filesystem::directory_iterator(dataPath)){
		std::string filePath = p.path();
		std::string fileName = p.path().filename();
		std::string level = fileName.substr(6, 1);
		std::string timestamp = fileName.substr(0, fileName.size() - 4);
		uint64_t timestampInt = std::stoull(timestamp);

		// Read the sstable
		SStable* newSSTable = new SStable(filePath);
		this->levelIndex[std::stoi(level)][timestampInt] = newSSTable;

		// Update the timestamp
		this->sstMaxTimeStamp = std::max(newSSTable->getSStableTimeStamp(), this->sstMaxTimeStamp);
	}
}

/**
 * Check if the sstable in each level needs to be merged
 */
uint64_t KVStore::mergeCheck(){
	// Check if the sstable in each level needs to be merged
	for(auto level = this->levelIndex.begin(); level != this->levelIndex.end(); level++){
		// Get the level
		int levelNum = level->first;

		// Check if the sstable in the level is full
		if(level->second.size() > level_max_file_num(levelNum)){
			return level->first;
		}
	}
	return -1;
}

/**
 * Merge the sstable in the given two levelw
 */
void KVStore::merge(uint64_t level){
	// Check if the target level exists
	if(level >= this->levelIndex.size()-1){
		// Create a new level
		std::string levelPath = this->SSTdir + "/level-" + std::to_string(level+1);
		utils::mkdir(levelPath.c_str());
	}
	
	/*
		Step1: select sstables in level X
	*/
	std::map<uint64_t, std::map<uint64_t, SStable*> > sstableSelect;

	if(level == 0){
		// Select the sstables in level 0
		for(auto sstable = this->levelIndex[0].begin(); sstable != this->levelIndex[0].end(); sstable++){
			sstableSelect[level][sstable->first] = sstable->second;
		}
	} else if(level > 0){
		uint64_t selectNum = levelIndex[level].size() - level_max_file_num(level);
		uint64_t alreadyChooseNum = 0;

		// Sort the sstables in the level by timestamp
		std::map<uint64_t, std::map<uint64_t, SStable*> > sortMap;
		std::map<uint64_t, std::map<uint64_t, uint64_t> > sstableName;

		for(auto sstable = this->levelIndex[level].begin(); sstable != this->levelIndex[level].end(); sstable++){
			// sstable->first = timestamp
			SStable *curTable = sstable->second;
			uint64_t curTimeStamp = curTable->getSStableTimeStamp();
			uint64_t minKey = curTable->getSStableMinKey();

			sortMap[curTimeStamp][minKey] = curTable;
			sstableName[curTimeStamp][minKey] = sstable->first;
		}

		for(auto iterX = sortMap.begin(); iterX != sortMap.end(); iterX++){
			for(auto iterY = iterX->second.begin(); iterY != iterX->second.end(); iterY++){
				if(alreadyChooseNum < selectNum){
					uint64_t curFileID = sstableName[iterX->first][iterY->first];

					sstableSelect[level][curFileID] = iterY->second;
					alreadyChooseNum++;
				}
			}
		}
	}

	/*
		Step2: select sstables in level X+1
	*/
	if(sstableSelect[level].size() > 0){
		// Get the minKey and maxKey of selected sstables in level X
		uint64_t LevelXminKey = UINT64_MAX;
		uint64_t LevelXmaxKey = 0;

		for(auto sstable = sstableSelect[level].begin(); sstable != sstableSelect[level].end(); sstable++){
			LevelXminKey = std::min(LevelXminKey, sstable->second->getSStableMinKey());
			LevelXmaxKey = std::max(LevelXmaxKey, sstable->second->getSStableMaxKey());
		}

		// Tranverse level X+1 to find the sstables that need to be merged
		for(auto iter = levelIndex[level+1].begin(); iter != levelIndex[level].end(); iter++){
			
			SStable *curTable = iter->second;
			uint64_t curMinKey = curTable->getSStableMinKey();
			uint64_t curMaxKey = curTable->getSStableMaxKey();

			// Insert to the sstableSelect if the key is covered
			if(curMinKey >= LevelXminKey && curMaxKey <= LevelXmaxKey){
				sstableSelect[level+1][iter->first] = curTable;
			}
		}
	}

	/*
		Step3: merge the sstables selected
	*/
	// mergeList[timestamp] = sstable*
	std::vector<SStable*> mergeList;

	// sortMap[key][timestamp] = {vlen, offset}
	std::map<uint64_t, std::map<uint64_t, std::map<uint64_t, uint32_t>> > sortMap;

	// Merge the sstables
	uint64_t WriteTimeStamp = 0;

	// Put the sstables in the mergeList and get the WriteTimeStamp
	for(auto iterX = sstableSelect.rbegin(); iterX != sstableSelect.rend(); iterX++){
		for(auto iterY = sstableSelect[iterX->first].rbegin(); iterY != sstableSelect[iterX->first].rend(); iterY++){
				SStable *curTable = iterY->second;
				mergeList.push_back(curTable);
				WriteTimeStamp = std::max(curTable->getSStableTimeStamp(), WriteTimeStamp);
		}
	}

	// Get all index entries in selected sstables
	for(uint64_t i = 0; i < sstableSelect.size(); i++){
		// iter[timestamp] = sstable*
		SStable *curtable = mergeList[i];
		uint64_t KVNum = curtable->getSStableKeyValNum();
		for(uint64_t i = 0; i < KVNum; i++){
			uint64_t curKey = curtable->getSStableKey(i);
			uint64_t curOffset = curtable->getSStableKeyOffset(i);
			uint32_t curVlen = curtable->getSStableKeyVlen(i);
			uint64_t curTimeStamp = curtable->getSStableTimeStamp();

			sortMap[curKey][curTimeStamp][curVlen] = curOffset;
		}
	}

	// Reprocess the sortMap to remove the deleted key
	std::map<uint64_t, std::map<uint64_t, std::map<uint64_t, uint32_t>> > sortMapProcessed;

	for(auto iterX = sortMap.begin(); iterX != sortMap.end(); iterX++){
		auto iterY = iterX->second.end();
		// iterY[timestamp] = {offset, vlen}
		iterY--;
		// iterZ[vlen] = offset
		auto iterZ = iterY->second.end();

		// Skip the deleted key in the bottom level
		if(iterZ->second == 0 && levelIndex[level+2].size() == 0)
			continue;

		// sortMapProcessed[key][timestamp][offset] = vlen
		sortMapProcessed[iterX->first][iterY->first][iterZ->first] = iterZ->second;
	}

	// Convert the sortMapProcessed to entriyMap
	std::map<uint64_t, std::map<uint64_t, uint32_t> > entriyMap;
	uint64_t listSSTfileSize = sstable_headerSize + sstable_bfSize;

	for(auto iter = sortMapProcessed.begin(); iter != sortMapProcessed.end(); iter++){
		uint64_t curKey = iter->first;
		uint64_t curOffset = iter->second.begin()->second.begin()->first;
		uint32_t curVlen = iter->second.begin()->second.begin()->second;

		// Update the new sstable file size
		uint64_t addSize = sstable_keySize + sstable_offsetSize + sstable_vlenSize;

		if(listSSTfileSize + addSize <= sstable_maxSize){
			listSSTfileSize += addSize;
			entriyMap[curKey][curOffset] = curVlen;
		} else{
			// Write the already stored entries into a new sstable
			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			std::chrono::microseconds nstime = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
			std::string newFilePath = this->SSTdir + "/level-" + std::to_string(level+1) + "/" + std::to_string(nstime.count()) + ".sst";

			SStable *newSSTable = new SStable(WriteTimeStamp, entriyMap, newFilePath, curvLogOffset);
			this->levelIndex[level+1][nstime.count()] = newSSTable;

			// Reset the entriyMap and listSSTfileSize
			entriyMap.clear();
			listSSTfileSize = sstable_headerSize + sstable_bfSize;
			entriyMap[curKey][curOffset] = curVlen;
		}
	}

	// Write the remaining entries into a new sstable
	if(entriyMap.size() > 0){
		// Generate the filename
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		std::chrono::microseconds nstime = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
		std::string newFilePath = this->SSTdir + "/level-" + std::to_string(level+1) + "/" + std::to_string(nstime.count()) + ".sst";

		SStable *newSSTable = new SStable(WriteTimeStamp, entriyMap, newFilePath, curvLogOffset);
		this->levelIndex[level+1][nstime.count()] = newSSTable;

		// Reset the entriyMap and listSSTfileSize
		entriyMap.clear();
		listSSTfileSize = sstable_headerSize + sstable_bfSize;
	}

	// Delete the selected old sstables in level X and X+1
	for(auto iterX = sstableSelect[level].begin(); iterX != sstableSelect[level].end(); iterX++){
		for(auto iterY = sstableSelect[iterX->first].begin(); iterY != sstableSelect[iterX->first].end(); iterY++){
			SStable *curtable = iterY->second;
			curtable->clear();

			if(curtable != NULL)
				delete curtable;
			if(levelIndex[iterX->first].count(iterY->first) == 1)
				levelIndex[iterX->first].erase(iterY->first);
		}
	}
}

/**
 * This reclaims space from vLog by moving valid value and discarding invalid value.
 * chunk_size is the size in byte you should AT LEAST recycle.
 */
void KVStore::gc(uint64_t chunk_size)
{
	
}
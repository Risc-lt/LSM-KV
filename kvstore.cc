#include "kvstore.h"
#include "config.h"
#include "sstable.h"
#include <cstdint>
#include <string>
#include <chrono>

KVStore::KVStore(const std::string &dir) : KVStoreAPI(dir)
{
	// Initialize the directory and timestamp
	this->dir = dir;
	this->sstMaxTimeStamp = 0;

	// Read all the sstables and write them into levelIndex
	this->sstFileCheck(dir);

	// Initialize the memtable
	this->memtable = new MemTable();

	// Initialize the vlog
	this->vlog = new vLog();

	// Initialize the vlog offset
	this->vlogOffset = 0;
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
		std::string newFilePath = this->dir + "/level-0/" + std::to_string(nstime.count()) + ".sst";
		SStable* newSSTable = new SStable(sstMaxTimeStamp, dataAll,  newFilePath, vlogOffset);

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
	// Check if the key is in the memtable
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
	std::string newFilePath = this->dir + "/level-0/" + std::to_string(mstime.count()) + ".sst";
	SStable* newSSTable = new SStable(sstMaxTimeStamp, dataAll,  newFilePath, vlogOffset);

	// Update the levelIndex
	this->levelIndex[0][mstime.count()] = newSSTable;

	// Add the key-value pair into the vlog
	this->vlog->readFromList(dataAll);
	this->vlogOffset = this->vlog->writeToFile(this->dir + "/vlog", this->vlogOffset);
	
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
				uint32_t indexRes = currentSSTable->getKeyIndexByKey(key);
				// If not found, continue
				if(indexRes == UINT32_MAX)
					continue;

				// Get the value
				std::string value = this->vlog->readValue(currentSSTable->getSStableKeyOffset(indexRes));
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

	// 
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	return false;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
}

/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 */
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list)
{
}

/**
 * This reclaims space from vLog by moving valid value and discarding invalid value.
 * chunk_size is the size in byte you should AT LEAST recycle.
 */
void KVStore::gc(uint64_t chunk_size)
{
}
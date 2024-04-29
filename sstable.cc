#include "sstable.h"
#include "config.h"
#include "utils.h"
#include <cstdint>

// Default Constructor
SStable::SStable(){};

// Constructor from file
SStable::SStable(std::string path){
    this->path = path;
    // Init the pointers
    this->header = new SSTheader(path, 0);
    this->bloomFliter = new BloomFilter<uint64_t, sstable_bfSize>(path, sstable_headerSize);
    this->index = new SSTIndex(path, sstable_headerSize + sstable_bfSize, header->keyValNum);

    // Read the file
    std::ifstream inFile(path, std::ios::binary | std::ios::in);

    // Get the size
    if(inFile){
        inFile.seekg(0, std::ios::end);
        this->fileSize = inFile.tellg();
        inFile.close();
    }
}

// Constructor from list
SStable::SStable(
    uint64_t setTimeStamp,
    std::list <std::pair<uint64_t, std::string> > &list,
    std::string setPath, uint64_t curvLogOffset){
    
    this->path = setPath;
    this->header = new SSTheader();
    this->bloomFliter = new BloomFilter<uint64_t, sstable_bfSize>();
    this->index = new SSTIndex();

    // Init the offset in vlog
    uint64_t vLogOffset = curvLogOffset;

    // Set the header and bloom filter
    this->header->timeStamp = setTimeStamp;
    uint64_t MinKey = UINT64_MAX;
    uint64_t MaxKey = 0;

    for(auto iter = list.begin(); iter != list.end(); iter++){
        // Update the MinKey and MaxKey
        MinKey = std::min(MinKey, iter->first);
        MaxKey = std::max(MaxKey, iter->first);

        // Insert the key and value
        this->bloomFliter->insert(iter->first);
        this->index->insert(iter->first, vLogOffset, iter->second.size());

        // Update the vLogOffset: Magic(1Byte) + Checksum(2Byte) + Key(8Byte) + vlen(4Byte) + Value
        vLogOffset += 15 + iter->second.size();
    }

    this->header->minKey = MinKey;
    this->header->maxKey = MaxKey;
    this->header->keyValNum = list.size();

    // Write the sstable to the file
    this->header->writeToFile(setPath, 0);
    this->bloomFliter->writeToFile(setPath, sstable_headerSize);
    this->index->writeToFile(setPath, sstable_headerSize + sstable_bfSize);

    // Read the file again
    std::ifstream inFile(setPath, std::ios::binary | std::ios::in);

    // Refresh the size
    if(inFile){
        inFile.seekg(0, std::ios::end);
        this->fileSize = inFile.tellg();
        inFile.close();
    }
}

// Destructor
SStable::~SStable(){
    delete this->header;
    delete this->bloomFliter;
    delete this->index;
}

// Clear all the data
void SStable::clear(){
    utils::rmfile(this->path.c_str());
}

// Get the target from the sstable
uint64_t SStable::getSStableTimeStamp(){
    return this->header->timeStamp;
}

uint64_t SStable::getSStableMinKey(){
    return this->header->minKey;
}

uint64_t SStable::getSStableMaxKey(){
    return this->header->maxKey;
}
    
uint64_t SStable::getSStableKeyValNum(){
    return this->header->keyValNum;
}

uint32_t SStable::getSStableKeyVlen(uint64_t index){
    return this->index->getVlen(index);
}
    
uint64_t SStable::getSStableKeyOffset(uint64_t index){
    return this->index->getOffset(index);
}

uint64_t SStable::getKeyIndexByKey(uint64_t key){
    return this->index->getIndex(key);
}

// Check if the key exists
bool SStable::checkIfKeyExist(uint64_t targetKey){
    // Check if the key is within the range
    if(targetKey < this->header->minKey || targetKey > this->header->maxKey)
        return false;

    // Check if the key exists in the bloom filter
    return this->bloomFliter->find(targetKey);
}

// // Scan the sstable
// std::list<std::pair<uint64_t, std::string>> SStable::scan(uint64_t key1, uint64_t key2){
//     // Store the pair into the list
//     std::list<std::pair<uint64_t, std::string>> SSTList;

//     // Get the start index of keys
//     uint32_t index1 = this->getKeyIndexByKey(key1);
//     uint32_t index2 = this->getKeyIndexByKey(key2);

//     // Check if the keys are within the range
//     if(index1 == UINT32_MAX || index2 == UINT32_MAX)
//         return SSTList;

//     // Traverse the sstable and get the key-value pairs
//     for(uint32_t i = 0; i <= this->getSStableKeyValNum(); i++){
//         // Get the key and value
//         uint64_t key = this->getSStableKey(i);
//         std::string value = this->getSStableValue(i);

//         // Check if the key is within the range
//         if(key >= key1 && key <= key2 && value != "")
//             SSTList.push_back(std::make_pair(key, value));
//     }

//     return SSTList;
// }
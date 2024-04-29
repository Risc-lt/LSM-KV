#include "sstindex.h"
#include <cstdint>

// Constructor
SSTIndex::SSTIndex(std::string path, uint64_t offset, size_t readKeyNum) {
    this->keyNum = 0;
    readFile(path, offset, readKeyNum);
}

// Load the sstindex from the file
int SSTIndex::readFile(std::string path, uint64_t offset, size_t readKeyNum) {
    std::ifstream inFile(path, std::ios::in | std::ios::binary);
    // Return -1 if the file is not opened
    if(inFile)
        return -1;

    // Read the key number
    inFile.seekg(0, std::ios::end);
    // Get the size of file
    uint64_t fileLimit = inFile.tellg();

    // Return -2 if the offset is out of range
    if(offset >= fileLimit || (offset + sizeof(uint64_t) * readKeyNum * 3) > fileLimit){
        inFile.close();
        return -2;
    }
    
    // Reset the file pointer
    inFile.clear();
    // Set the file pointer to the offset
    inFile.seekg(offset, std::ios::beg);

    // Read and store the keys, offsets, and value lengths
    for(size_t i = 0; i < readKeyNum; i++){
        uint64_t key, offset, vlen;

        inFile.read((char*)&key, sizeof(uint64_t));
        inFile.read((char*)&offset, sizeof(uint64_t));
        inFile.read((char*)&vlen, sizeof(uint64_t));
        
        keyVec.push_back(key);
        offsetVec.push_back(offset);
        vlenVec.push_back(vlen);
        keyNum++;
    }

    inFile.close();
    return 0;
}

// Write the sstindex to the file
uint64_t SSTIndex::writeToFile(std::string path, uint64_t offset) {
    bool ifFileExist = false;
    uint64_t fileLimit = 0;
    std::ifstream inFile(path, std::ios::in | std::ios::binary);

    // Check if the file exists
    if(inFile){
        ifFileExist = true;
        inFile.seekg(0, std::ios::end);
        fileLimit = inFile.tellg();
        inFile.close();
    }

    // Create a new file if the file does not exist
    if(!ifFileExist){
        offset = 0;
        std::ofstream createFile(path, std::ios::out | std::ios::binary);
        createFile.close();
    } else if(ifFileExist && (offset > fileLimit)){
        offset = fileLimit;
    }

    std::fstream outFile(path, std::ios::out | std::ios::in | std::ios::binary);

    if(!outFile)
        return -1;

    // Set the file pointer to the offset
    outFile.seekp(offset, std::ios::beg);

    // Write the keys, offsets, and value lengths to the file
    for(size_t i = 0; i < keyNum; i++){
        outFile.write((char*)&keyVec[i], sizeof(uint64_t));
        outFile.write((char*)&offsetVec[i], sizeof(uint64_t));
        outFile.write((char*)&vlenVec[i], sizeof(uint64_t));
    }

    outFile.close();
    return offset;
}

// Insert a <key, offset, vlen> into the vector
void SSTIndex::insert(uint64_t newKey, uint64_t newOffset, uint64_t newVlen) {
    keyVec.push_back(newKey);
    offsetVec.push_back(newOffset);
    vlenVec.push_back(newVlen);
    keyNum++;
}

// Get key by index
uint64_t SSTIndex::getKey(uint64_t index) {
    if(index >= keyNum){
        exit(-1);
        return -1;
    }
    return keyVec[index];
}

// Get offset by index
uint64_t SSTIndex::getOffset(uint64_t index) {
    if(index >= keyNum){
        exit(-1);
        return -1;
    }
    return offsetVec[index];
}

// Get value length by index
uint64_t SSTIndex::getVlen(uint64_t index) {
    if(index >= keyNum){
        exit(-1);
        return -1;
    }
    return vlenVec[index];
}

// Get index by key
uint64_t SSTIndex::getIndex(uint64_t key) {
    if(this->keyNum == 0)
        return UINT64_MAX;

    // keyVec is strictly increasing because the MemTable is sorted when transformed to SSTable
    if(key < keyVec[0])
        return 0;
    if(key > keyVec[keyNum - 1])
        return UINT64_MAX;

    // Binary search
    uint64_t left = 0, right = keyVec.size() - 1;
    uint64_t mid = 0;
    uint64_t findIndex = UINT64_MAX;
    bool ifFind = false;
    while(left <= right){
        mid = left + ((right - left) / 2);
        if(keyVec[mid] == key){
            findIndex = mid;
            ifFind = true;
            break;
        } else if(keyVec[mid] < key){
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    } 

    if(ifFind)
        return findIndex;
    else
        return UINT64_MAX;
}
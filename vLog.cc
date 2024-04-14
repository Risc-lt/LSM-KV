#include "vLog.h"
#include <cstdint>
#include <fstream>

// Write the vLog to a file
uint64_t vLog::writeToFile(std::string path, uint64_t offset){
    bool ifFileExists = false;
    uint64_t fileSize = 0;
    std::ifstream inFile(path, std::ios::in | std::ios::binary);
   
    // Check if the file exists
    if(inFile){
        ifFileExists = true;
        inFile.seekg(0, std::ios::end);
        fileSize = inFile.tellg();
        inFile.close();
    }

    // If the file does not exist, create it
    if(!ifFileExists){
        offset = 0;
        std::ofstream outFile(path, std::ios::out | std::ios::binary);
        outFile.close();
    } else if(ifFileExists && (offset > fileSize)){
        offset = fileSize;
    }

    // Open the file for writing
    std::fstream outFile(path, std::ios::out | std::ios::in | std::ios::binary);

    if(!outFile)
        return -1;

    // Seek to the offset
    outFile.seekp(offset, std::ios::beg);

    for(size_t i = 0; i < this->entries.size(); i++){
        vLogEntry entry = this->entries[i];
        
        // Write the entry to the file
        outFile.write((char*)&entry.Magic, sizeof(uint8_t));
        outFile.write((char*)&entry.Checksum, sizeof(uint16_t));
        outFile.write((char*)&entry.Key, sizeof(uint64_t));
        outFile.write((char*)&entry.vlen, sizeof(uint32_t));
        outFile.write((char*)&entry.Value, entry.Value.size());
    }

    outFile.close();

    //  Clear the entries
    this->entries.clear();

    return offset;
}

// Insert a new value
void vLog::insert(uint64_t Key, std::string newVal){
    // Create a new vLogEntry
    vLogEntry entry(Key, newVal);

    // Add the entry to the entries vector
    this->entries.push_back(entry);
    this->valNum++;
}

// Get the value at the index
std::string vLog::getVal(size_t index){
    if(index > this->valNum)
        return sstvalue_outOfRange;

    return this->entries[index].Value;
}

// Get the value from a file
std::string vLog::getValFromFile(std::string path, uint64_t offset, size_t length){
    std::ifstream inFile(path, std::ios::in | std::ios::binary);
    if(!inFile)
        return sstvalue_readFile_file;

    // Move fp to the end
    inFile.seekg(0, std::ios::end);
    // Get the file size
    size_t fileSize = inFile.tellg();

    // Check if the offset is out of range
    if(offset > fileSize || (offset + length) > fileSize){
        inFile.close();
        return sstvalue_outOfRange;
    }
    
    // Move fp to the offset
    inFile.seekg(offset, std::ios::beg);

    // Read the value
    char* buffer = new char[length];
    inFile.read(buffer, length);

    // Copy the buffer to a string for free
    std::string res = "";
    for(size_t i = 0; i < length; i++)
        res += buffer[i];

    inFile.close();

    // Free the buffer
    delete[] buffer;

    return res;
}
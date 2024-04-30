#include "vLog.h"
#include <cstdint>
#include <fstream>

// Constructor
vLog::vLog(std::string path){
    this->path = path;
    this->tail = 0;
    this->head = 0;
}

// Write the vLog to a file
uint64_t vLog::writeToFile(uint64_t offset){
    bool ifFileExists = false;
    uint64_t fileSize = 0;
    std::ifstream inFile(this->path, std::ios::in | std::ios::binary);
   
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

    // Update the offset
    this->head = outFile.tellp();

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
    head++;
}

// Get the value from a file
std::string vLog::getValFromFile(std::string path, uint64_t offset, uint32_t length){
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

    // Check if the offset is within the valid range
    if(offset <= tail || offset >= head){
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

// Read the key from a file
uint64_t vLog::getKeyFromFile(std::string path, uint64_t offset){
    std::ifstream inFile(path, std::ios::in | std::ios::binary);
    if(!inFile)
        return -1;

    // Move fp to the end
    inFile.seekg(0, std::ios::end);
    // Get the file size
    size_t fileSize = inFile.tellg();

    // Check if the offset is out of range
    if(offset > fileSize){
        inFile.close();
        return -1;
    }

    // Check if the offset is within the valid range
    if(offset <= tail || offset >= head){
        inFile.close();
        return -1;
    }
    
    // Move fp to the offset
    // Key(8 Byte)->vlen(4 Byte)->Value
    inFile.seekg(offset-12, std::ios::beg);

    // Read the value
    char* buffer = new char[sizeof(uint64_t)];
    inFile.read(buffer, sizeof(uint64_t));

    // Convert the buffer to uint64_t
    uint64_t res = atoi(buffer);

    inFile.close();

    // Free the buffer
    delete[] buffer;

    return res;
    
}

// Read the vlen from a file
uint32_t vLog::getVlenFromFile(std::string path, uint64_t offset){
    std::ifstream inFile(path, std::ios::in | std::ios::binary);
    if(!inFile)
        return -1;

    // Move fp to the end
    inFile.seekg(0, std::ios::end);
    // Get the file size
    size_t fileSize = inFile.tellg();

    // Check if the offset is out of range
    if(offset > fileSize){
        inFile.close();
        return -1;
    }

    // Check if the offset is within the valid range
    if(offset <= tail || offset >= head){
        inFile.close();
        return -1;
    }
    
    // Move fp to the offset
    // Key(8 Byte)->vlen(4 Byte)->Value
    inFile.seekg(offset-4, std::ios::beg);

    // Read the value
    char* buffer = new char[sizeof(uint64_t)];
    inFile.read(buffer, sizeof(uint64_t));

    // Convert the buffer to uint64_t
    uint32_t res = atoi(buffer);

    inFile.close();

    // Free the buffer
    delete[] buffer;

    return res;
    
}

// Read Magic from a file
uint8_t vLog::getMagicFromFile(std::string path, uint64_t offset){
    std::ifstream inFile(path, std::ios::in | std::ios::binary);
    if(!inFile)
        return -1;

    // Move fp to the end
    inFile.seekg(0, std::ios::end);
    // Get the file size
    size_t fileSize = inFile.tellg();

    // Check if the offset is out of range
    if(offset > fileSize){
        inFile.close();
        return -1;
    }

    // Check if the offset is within the valid range
    if(offset <= tail || offset >= head){
        inFile.close();
        return -1;
    }
    
    // Move fp to the offset
    // Magic(1 Byte) -> Checksum(2 Byte) -> Key(8 Byte) -> vlen(4 Byte) -> Value
    inFile.seekg(offset-15, std::ios::beg);

    // Read the value
    char* buffer = new char[sizeof(uint64_t)];
    inFile.read(buffer, sizeof(uint64_t));

    // Convert the buffer to uint64_t
    uint32_t res = atoi(buffer);

    inFile.close();

    // Free the buffer
    delete[] buffer;

    return res;
    
}

// Read Checksum from a file
uint16_t vLog::getChecksumFromFile(std::string path, uint64_t offset){
    std::ifstream inFile(path, std::ios::in | std::ios::binary);
    if(!inFile)
        return -1;

    // Move fp to the end
    inFile.seekg(0, std::ios::end);
    // Get the file size
    size_t fileSize = inFile.tellg();

    // Check if the offset is out of range
    if(offset > fileSize){
        inFile.close();
        return -1;
    }

    // Check if the offset is within the valid range
    if(offset <= tail || offset >= head){
        inFile.close();
        return -1;
    }
    
    // Move fp to the offset
    // Magic(1 Byte) -> Checksum(2 Byte) -> Key(8 Byte) -> vlen(4 Byte) -> Value
    inFile.seekg(offset-14, std::ios::beg);

    // Read the value
    char* buffer = new char[sizeof(uint64_t)];
    inFile.read(buffer, sizeof(uint64_t));

    // Convert the buffer to uint64_t
    uint32_t res = atoi(buffer);

    inFile.close();

    // Free the buffer
    delete[] buffer;

    return res;
    
}

// Read the vLog from a list
void vLog::readFromList(std::list<std::pair<uint64_t, std::string>> list){
    for(auto iter = list.begin(); iter != list.end(); iter++){
        // Skip empty values
        if(iter->second.size() == 0)
            continue;

        // Create a new vLogEntry
        vLogEntry entry(iter->first, iter->second);
        this->entries.push_back(entry);
    }
}
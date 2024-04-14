#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

class SSTIndex {
private:
    uint64_t keyNum;
    std::vector<uint64_t> keyVec;
    std::vector<uint64_t> offsetVec;
    std::vector<uint64_t> vlenVec;

public:
    // Constructor
    SSTIndex(){keyNum = 0;};
    SSTIndex(std::string path, uint64_t offset, size_t readKeyNum);

    // Destructor
    ~SSTIndex(){};

    // Get the number of keys
    uint64_t getKeyNum() {return this->keyNum;};
    // Get the key from the vector
    uint64_t getKey(uint64_t index);
    // Get the offset from the vector
    uint64_t getOffset(uint64_t index);
    // Get the value length from the vector
    uint64_t getVlen(uint64_t index);
    // Get the index of the key
    uint64_t getIndex(uint64_t key);

    // Insert a <key, offset, vlen> into the vector
    void insert(uint64_t newKey, uint64_t newOffset, uint64_t newVlen);

    // Load the sstindex from the file
    int readFile(std::string path, uint64_t offset, size_t readKeyNum);
    // Write the sstindex to the file
    uint64_t writeToFile(std::string path, uint64_t offset);
};
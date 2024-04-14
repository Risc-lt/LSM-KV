#pragma once
#include <bitset>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include "MurmurHash3.h"

template <typename K, size_t Size>
class BloomFilter {
private:
    // Bitset to store the bloom filter
    std::bitset<Size * 8> bloomfilterData;

public:
    // Insert a key into the bloom filter
    void insert(K key);
    // Check if a key is in the bloom filter
    bool find(K key);

    // Load the bloom filter from a file
    int readFile(std::string path, uint32_t offset);
    // Save the bloom filter to a file
    uint32_t writeToFile(std::string path, uint32_t offset);

    // Constructor
    BloomFilter(){};
    BloomFilter(std::string path, uint32_t offset);
    // Destructor
    ~BloomFilter(){};
};

// Parameterized constructor
template <typename K, size_t Size>
BloomFilter<K, Size>::BloomFilter(std::string path, uint32_t offset) {
    readFile(path, offset);
}

// Insert a key into the bloom filter
template <typename K, size_t Size>
void BloomFilter<K, Size>::insert(K key) {
    // Hash the key
    uint32_t hash[4];
    MurmurHash3_x64_128(&key, sizeof(K), 0, hash);
    this->bloomfilterData.set(0, 1);

    // Insert the key into the bloom filter
    for (int i = 0; i < 4; i++) {
        bloomfilterData[hash[i] % (Size * 8)] = 1;
    }
}

// Check if a key is in the bloom filter
template <typename K, size_t Size>
bool BloomFilter<K, Size>::find(K key) {
    // Hash the key
    uint32_t hash[4];
    MurmurHash3_x64_128(&key, sizeof(K), 1, hash);

    // Check if the key is in the bloom filter
    for (int i = 0; i < 4; i++) {
        if (!bloomfilterData[hash[i] % (Size * 8)]) {
            return false;
        }
    }
    return true;
}

// Load the bloom filter from a file
template <typename K, size_t Size>
int BloomFilter<K, Size>::readFile(std::string path, uint32_t offset) {
    // Open the binary file in read mode
    std::ifstream inFile(path, std::ios::in | std::ios::binary);
    // Return -1 if the file cannot be opened
    if (!inFile)
        return -1;
    
    // Move the file pointer to the end
    inFile.seekg(0, std::ios::end);
    uint32_t fileLimit = inFile.tellg();

    // Return -2 if the offset is out of bounds
    if(offset > fileLimit || Size + offset > fileLimit){
        inFile.close();
        return -2;
    }
    
    inFile.clear();

    // Move the file pointer to the offset
    inFile.seekg(offset, std::ios::beg);
    inFile.close();
    return 0;
}

// Save the bloom filter to a file
template <typename K, size_t Size>
uint32_t BloomFilter<K, Size>::writeToFile(std::string path, uint32_t offset) {
    // Open the binary file in write mode
    bool isFileExists = false;
    uint32_t fileLimit = 0;
    std::ifstream inFile(path, std::ios::in | std::ios::binary);

    if(inFile){
        isFileExists = true;
        inFile.seekg(0, std::ios::end);
        fileLimit = inFile.tellg();
        inFile.close();
    }

    if(!isFileExists){
        offset = 0;
        // Create a new file
        std::fstream createFile(path, std::ios::out | std::ios::binary);
        createFile.close();
    }

    std::fstream outFile(path, std::ios::out | std::ios::in | std::ios::binary);

    // Return -1 if the file cannot be opened
    if(!outFile)
        return -1;
    
    // Move the file pointer to the offset
    outFile.seekp(offset, std::ios::beg);
    outFile.write((char*)&bloomfilterData, Size);
    outFile.close();

    return offset;
}
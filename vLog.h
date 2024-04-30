#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include "config.h"
#include "utils.h"

class vLogEntry{
public:
    // vLog entry fields
    uint8_t Magic;
    uint16_t Checksum;
    uint64_t Key;
    uint32_t vlen;
    std::string Value;

    // Default constructor
    vLogEntry(){Magic = 0xff;};

    // Parameterized constructor
    vLogEntry(uint64_t key, std::string value){
    this->Magic = 0xff;
    this->Key = key;
    this->Value = value;
    this->vlen = value.size();

    std::vector<unsigned char> data;

    // Add Key to data
    unsigned char* keyPtr = reinterpret_cast<unsigned char*>(&this->Key);
    for (size_t i = 0; i < sizeof(this->Key); ++i) {
        data.push_back(keyPtr[i]);
    }

    // Add vlen to data
    unsigned char* vlenPtr = reinterpret_cast<unsigned char*>(&this->vlen);
    for (size_t i = 0; i < sizeof(this->vlen); ++i) {
        data.push_back(vlenPtr[i]);
    }

    // Add Value to data
    for (const char& c : this->Value) {
        data.push_back(static_cast<unsigned char>(c));
    }

    this->Checksum = utils::crc16(data);
};

    // Destructor
    ~vLogEntry(){};
};

class vLog{
private:
    std::string path;
    uint64_t tail, head;
    std::vector<vLogEntry> entries;

public:
    // Get the number of values in the vLog
    uint64_t getHead(){return head;};
    uint64_t getTail(){return tail;};

    // Get the vLog path
    std::string getPath(){return path;};

    // Insert a new value
    void insert(uint64_t Key, std::string newVal);

    // Write the vLog to a file
    uint64_t writeToFile(uint64_t offset);

    // Read the value from a file
    std::string getValFromFile(std::string path, uint64_t offset, uint32_t length);
    // Read the key from a file
    uint64_t getKeyFromFile(std::string path, uint64_t offset);
    // Read the vlen from a file
    uint32_t getVlenFromFile(std::string path, uint64_t offset);
    // Read Magic from a file
    uint8_t getMagicFromFile(std::string path, uint64_t offset);
    // Read Checksum from a file
    uint16_t getChecksumFromFile(std::string path, uint64_t offset);

    // Read the vLog from a list
    void readFromList(std::list<std::pair<uint64_t, std::string>>);

    // Default constructor and destructor
    vLog(std::string path);
    ~vLog(){};
};
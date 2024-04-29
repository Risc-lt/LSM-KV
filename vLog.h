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
    uint64_t valNum;
    std::vector<vLogEntry> entries;

public:
    //  Get the number of values in the vLog
    size_t getValNum() {return this->valNum;};

    // Get the value at the index
    std::string getValbyOffset(uint32_t offset);

    // Insert a new value
    void insert(uint64_t Key, std::string newVal);

    // Write the vLog to a file
    uint64_t writeToFile(std::string path, uint64_t offset);
    // Read the vLog from a file
    std::string getValFromFile(std::string path, uint64_t offset, size_t length);
    // Read the vLog from a list
    void readFromList(std::list<std::pair<uint64_t, std::string>>);

    // Default constructor and destructor
    vLog(){valNum = 0;};
    ~vLog(){};
};
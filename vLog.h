#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
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
        this->Checksum = utils::crc16(value.c_str(), value.size());
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
    std::string getVal(size_t index);

    // Insert a new value
    void insert(uint64_t Key, std::string newVal);

    // Write the vLog to a file
    uint64_t writeToFile(std::string path, uint64_t offset);
    // Read the vLog from a file
    std::string getValFromFile(std::string path, uint64_t offset, size_t length);

    // Default constructor and destructor
    vLog(){valNum = 0;};
    ~vLog(){};
};
#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>

class SSTheader {
public:
    uint64_t timeStamp;
    uint64_t keyValNum;
    uint64_t minKey;
    uint64_t maxKey;

    // Constructor
    SSTheader(){};
    SSTheader(std::string path, uint64_t offset);

    // Destructor
    ~SSTheader(){};

    // Load the SST header from a file
    int readFile(std::string path, uint64_t offset);

    // Save the SST header to a file
    uint64_t writeToFile(std::string path, uint64_t offset);
};
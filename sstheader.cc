#include "sstheader.h"

// Constructor
SSTheader::SSTheader(std::string path, uint64_t offset) {
    if(readFile(path, offset) != 0) {
        std::cerr << "[Error] Failure of constructing sstheader" << std::endl;
    }
}

// Load the SST header from a file
int SSTheader::readFile(std::string path, uint64_t offset) {
    std::ifstream inFile(path, std::ios::in | std::ios::binary);
    if(!inFile)
        return -1;

    // Move the file pointer to the offset
    inFile.seekg(0,std::ios::end);
    // Get the file size
    uint64_t fileLimit = inFile.tellg();

    // Check if the offset is out of bounds
    if(offset > fileLimit || offset + sizeof(SSTheader) > fileLimit){
        inFile.close();
        return -2;
    }
    
    inFile.clear();
    
    // Move the file pointer to the offset
    inFile.seekg(offset,std::ios::beg);

    // Read the SST header
    inFile.read((char*)&timeStamp, sizeof(timeStamp));
    inFile.read((char*)&keyValNum, sizeof(keyValNum));
    inFile.read((char*)&minKey, sizeof(minKey));
    inFile.read((char*)&maxKey, sizeof(maxKey));

    // Close the file
    inFile.close();
    return 0;
}

// Save the SST header to a file
uint64_t SSTheader::writeToFile(std::string path, uint64_t offset) {
    bool isFileExists = false;
    uint64_t fileLimit = 0;
    std::ifstream inFile(path, std::ios::in | std::ios::binary);

    // Check if the file exists
    if(inFile){
        isFileExists = true;
        inFile.seekg(0, std::ios::end);
        fileLimit = inFile.tellg();
        inFile.close();
    }

    // Create a new file if it does not exist
    if(!isFileExists){
        offset = 0;
        std::fstream createFile(path, std::ios::out | std::ios::binary);
        createFile.close();
    } else if (isFileExists && offset > fileLimit) {
        // If the offset is out of bounds, set the offset to the end of the file
        offset = fileLimit;
    }

    // Open the file in read/write mode
    std::fstream outFile(path, std::ios::out | std::ios::in | std::ios::binary);

    if(!outFile)
        return -1;

    // Move the file pointer to the offset
    outFile.seekp(offset, std::ios::beg);

    // Write the SST header
    outFile.write((char*)&timeStamp, sizeof(timeStamp));
    outFile.write((char*)&keyValNum, sizeof(keyValNum));
    outFile.write((char*)&minKey, sizeof(minKey));
    outFile.write((char*)&maxKey, sizeof(maxKey));

    // Close the file
    outFile.close();
    return offset;
}
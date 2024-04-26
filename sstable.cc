#include "sstable.h"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include "util.h"

// Default Constructor
SStable::SStable(){};

// Constructor from file
SStable::SStable(std::string path){
    this->path = path;
    // Init the pointers
    this->header = nullptr;
    this->bloomFliter = nullptr;
    this->index = nullptr;

    // Read the file
    std::ifstream inFile(path, std::ios::binary | std::ios::in);

    // Get the size
    if(inFile){
        inFile.seekg(0, std::ios::end);
        this->fileSize = inFile.tellg();
        inFile.close();
    }
}

// Constructor from list
SStable::SStable(
    uint64_t setTimeStamp,
    std::list <std::pair<uint64_t, std::string> > &list,
    std::string setPath){
    
    this->path = setPath;
    this->header = new SSTheader();
    this->bloomFliter = new BloomFilter<uint64_t, sstable_bfSize>();
    this->index = new SSTIndex();

    // Set the header
    this->header->timeStamp(setTimeStamp);

}
#pragma once

#include "sstheader.h"
#include "bloomfilter.h"
#include "sstindex.h"
#include "vLog.h"
#include "config.h"
#include <string>
#include <cstdint>
#include <list>
#include <map>

class SStable{
private:
    // Data part
    std::string path;           // Path of the sstable file
    uint64_t fileSize;          // Size of the sstable file

    // Consruction part
    SSTheader * header = NULL;
    BloomFilter<uint64_t, sstable_bfSize > * bloomFliter = NULL;
    SSTIndex * index = NULL;

public:
    
    // Init a SStable from a file
    SStable(std::string path);

    // Init a SStable from a list
    SStable(uint64_t setTimeStamp, 
        std::list <std::pair<uint64_t, std::string> > &list,
        std::string setPath, uint64_t curvLogOffset);

    // Clear all the data
    void clear();

    // Get the target from the sstable
    uint64_t getSStableTimeStamp();
    uint64_t getSStableMinKey();
    uint64_t getSStableMaxKey();
    uint64_t getSStableKeyValNum();

    uint32_t getSStableKeyVlen(size_t index);
    uint64_t getSStableKeyOffset(size_t index);
    uint64_t getKeyIndexByKey(uint64_t key);
    // std::string getSStableValue(size_t index);

    bool checkIfKeyExist(uint64_t targetKey);

    // std::list<std::pair<uint64_t, std::string>> scan(uint64_t key1, uint64_t key2);

    SStable();
    ~SStable();
};
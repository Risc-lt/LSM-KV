#pragma once

#include "sstheader.h"
#include "bloomfilter.h"
#include "sstindex.h"
#include "vlog.h"
#include "config.h"
#include <list>
#include <map>

class SStable{
private:
    // Data part
    std::string path;           // Path of the sstable file
    uint32_t fileSize;          // Size of the sstable file
    bool cachePolicy[4];        // Cache status of the sstable

    // Consruction part
    SSTheader * header = NULL;
    BloomFilter<uint64_t, sstable_bfSize > * bloomFliter = NULL;
    SSTIndex * index = NULL;

    // Inner fuctions
    SSTheader * getHeaderPtr();
    BloomFilter<uint64_t, sstable_bfSize >* getBloomFliterPtr();
    SSTIndex * getIndexPtr();
    vLogEntry * getValuePtr();

    // Save cache status in case of being changed subtly
    void saveCachePolicy(bool (&saveTarget)[4]);

public:

    void refreshCachePolicy(bool setCachePolicy[4]);
    
    // Init a SStable from a file
    SStable(std::string path, bool cachePolicy[4]);

    // Init a SStable from a list
    SStable(uint64_t setTimeStamp, 
        std::list <std::pair<uint64_t, std::string> > &list,
        std::string setPath, bool cachePolicy[4]);
    

    // Test function
    void devTest();

    // Clear the cache
    void clear();

    // Get the value of the key from the sstable
    uint64_t getSStableTimeStamp();
    uint64_t getSStableMinKey();
    uint64_t getSStableMaxKey();
    uint64_t getSStableKeyValNum();

    uint64_t getSStableKey(size_t index);
    uint32_t getSStableKeyOffset(size_t index);
    uint32_t getKeyIndexByKey(uint64_t key);
    uint32_t getKeyOrLargerIndexByKey(uint64_t key);
    std::string getSStableValue(size_t index);


    bool checkIfKeyExist(uint64_t targetKey);

    void scan(uint64_t key1, uint64_t key2, std::map<uint64_t, std::map<uint64_t, std::string> >  &scanMap);


    SStable();
    ~SStable();
};
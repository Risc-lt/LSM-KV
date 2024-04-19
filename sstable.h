#pragma once

#include "sstheader.h"
#include "bloomfilter.h"
#include "sstindex.h"
#include "config.h"
#include <list>
#include <map>
#include <string>

class SStable{
private:
    // Date Part
    std::string path;   // Path to the sstable file
    uint64_t fileSize;  // Size of the sstable file
    bool cachePolicy;   // Cache status of the sstable file

    // Congiguration Part
    
};  
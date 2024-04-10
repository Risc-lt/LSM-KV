#pragma once
#include "skiplist.h"

class MemTable {
public:
    // Use skiplist as container
    Skiplist<uint64_t, std::string>* skiplist;

};
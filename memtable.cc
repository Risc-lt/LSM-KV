#include "memtable.h"
#include <cstdint>

// Constructor
MemTable::MemTable() {
    // Initialize the memtable
    skiplist = new Skiplist<uint64_t, std::string>();
    // Record the size
    size = 0;

}
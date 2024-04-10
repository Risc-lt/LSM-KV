#pragma once
#include "skiplist.h"
#include <cstdint>
#include <list>

class MemTable {
private:
    // Use skiplist as container
    Skiplist<uint64_t, std::string>* skiplist;
    // Size of memtable when transferred to sstable
    size_t size;

    // Internal functions
    void putKV(uint64_t key, const std::string &s);
    bool delKV(uint64_t key);

    // Introduce log for debug
    void writeLog(std::string path, int operationID, uint64_t key, std::string value);
    void restoreFromLog(std::string path);

public:
    MemTable();
    ~MemTable();

    // Check if memtable is full
    bool putCheck(uint64_t key, const std::string &s);

    // Put key-value pair into memtable
    void put(uint64_t key, const std::string &s);

    // Get value by key
    std::string get(uint64_t key);

    // Delete key-value pair by key
    bool del(uint64_t key);

    // Clear all key-value pairs in memtable
    void reset();

    // Scan key-value pairs in memtable
    void scan(uint64_t key1, uint64_t key2, std::map<uint64_t, std::string> &resultMap);

    // Copy all key-value pairs in memtable to sstable
    void copyAll(std::list<std::pair<uint64_t, std::string>> &list);

    // Print all key-value pairs in memtable
    void tranverse(){this->skiplist->tranverse();};
    
};
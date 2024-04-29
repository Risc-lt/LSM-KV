#include "memtable.h"
#include <cstddef>
#include <cstdint>
#include <string>

const int log_putID = 0;
const int log_delID = 1;
const std::string log_putStr = "PUT";
const std::string log_delStr = "DEL"; 

// Constructor
MemTable::MemTable() {
    // Initialize the memtable
    skiplist = new Skiplist<uint64_t, std::string>();
    // Record the size
    sstSpaceSize = sstable_headerSize + sstable_bfSize;

    // Create log
    this->restoreFromLog(logFilePath);
}

// Destructor
MemTable::~MemTable() {
    utils::rmdir(logFilePath);
    delete skiplist;
    skiplist = nullptr;
}

/****************************************************************************************
 **                                 Private functions                                  **
 ****************************************************************************************/

// Insert key-value pair into memtable
void MemTable::putKV(uint64_t key, const std::string &s) {
    // Check if the key already exists
    Node<uint64_t, std::string> *tryFind = skiplist->find(key);

    // If the key exists, update the value
    if (tryFind != nullptr) {
        // Update the value
        this->skiplist->insert(key, s);
        // Modify the size of memtable
        if(s.size() > tryFind->value.size()) {
            sstSpaceSize += s.size() - tryFind->value.size();
        } else {
            sstSpaceSize -= tryFind->value.size() - s.size();
        }
    } else {
        // If the key does not exist, insert the key-value pair
        this->skiplist->insert(key, s);
        // Enlarge the size of memtable
        sstSpaceSize += sstable_keySize + sstable_offsetSize + s.size();
    }
}

// Delete key-value pair by key
bool MemTable::delKV(uint64_t key) {
    // Check if the key exists
    auto tryFind = this->skiplist->find(key);

    // If the key don't exist, return false
    if (tryFind == nullptr)
        return false;
    
    // If the key exists, modify the value to be deleted
    tryFind->value = delete_tag;
    // Update the size
    if(sizeof(delete_tag) > tryFind->value.size())
        sstSpaceSize += sizeof(delete_tag) - tryFind->value.size();
    else 
        sstSpaceSize -= tryFind->value.size() - sizeof(delete_tag);
    return true;
}

/****************************************************************************************
 **                                 Public functions                                   **
 ****************************************************************************************/

// Interface: PUT(Key, Value)
void MemTable::put(uint64_t key, const std::string &s) {
    // Insert the key-value pair into memtable
    this->putKV(key, s);
    // Write log
    this->writeLog(logFilePath, log_putID, key, s);
}

// Interface: DEL(Key)
bool MemTable::del(uint64_t key) {
    // Write log
    this->writeLog(logFilePath, log_delID, key, "");
    // Return whether successfilly delete the key-value pair
    return (this->delKV(key));
}

// Interface: GET(Key)
std::string MemTable::get(uint64_t key) {
    // Find the key-value pair by key
    auto tryFind = this->skiplist->find(key);

    // If the key exists, return the value
    if (tryFind != nullptr) {
        std::string res = tryFind->value;
        // Check if the value is already deleted
        if(res == delete_tag)
            return memtable_already_deleted;
        return res;
    } 

    // If the key does not exist, return empty string
    return memtable_not_exist;
}

// Interface: RESET()
void MemTable::reset() {
    // Clear all logs before
    utils::rmdir(logFilePath);
    // Clear all key-value pairs in memtable
    this->skiplist->clear();
    // Reset the size of memtable
    sstSpaceSize = sstable_headerSize + sstable_bfSize;
}

// Interface: SCAN(Key1, Key2)
void MemTable::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) {
    // Start scanning from key1
    Node<uint64_t, std::string> *iter = this->skiplist->find(key1);
    
    // Traverse the skiplist
    while (iter->type == nodeType_Data && iter->key <= key2) {
        // Check if the value is already deleted
        if(iter->value != delete_tag)
            list.push_back(std::make_pair(iter->key, iter->value));
        iter = iter->next[0];
    }
    return;
}

// Check if the memtable is full before inserting key-value pair
bool MemTable::putCheck(uint64_t key, const std::string &s) {
    // New size after inserting the key-value pair
    size_t newSize = sstable_keySize + sstable_offsetSize + s.size();

    Node<uint64_t, std::string> *tryFind = skiplist->find(key);

    // If the key don't exist, add the newSize and check whether out of limitation
    if (tryFind == nullptr) {
        return (sstSpaceSize + newSize <= sstable_maxSize);
    } else {
        // If the key exists, check whether the new size is larger than the old size
        if (newSize > tryFind->value.size()) {
            return (sstSpaceSize + newSize - tryFind->value.size() <= sstable_maxSize);
        } else {
            return true;
        }
    }
}

// Copy the key-value pairs in memtable to sstable
std::list<std::pair<uint64_t, std::string>> MemTable::copyAll() {
    std::list<std::pair<uint64_t, std::string>> list;
    this->skiplist->copyAll(list);
    return list;
}

// Write log
void MemTable::writeLog(std::string path, int operationID, uint64_t key, std::string value) {
    // Open the log file
    std::ofstream outFile(path, std::fstream::out | std::fstream::app);

    switch (operationID) {
        case log_putID:
            outFile << log_putStr << " " << key << " " << value << std::endl;
            break;
        case log_delID:
            outFile << log_delStr << " " << key << std::endl;
            break;
        default:
            break;
    }

    // Close the log file
    outFile.close();
}

// Restore from log
void MemTable::restoreFromLog(std::string path) {
    // Open the log file
    std::ifstream inFile;
    inFile.open(path);

    std::string rowLog;
    std::string operationType;
    uint64_t key;
    std::string value;

    if(inFile.is_open()) {
        while(getline(inFile, rowLog)) {
            std::vector<size_t> spaceIndex;

            // scan for the first two spaces
            for(size_t i = 0; i < rowLog.size(); i++){
                if(rowLog[i] == ' '){
                    spaceIndex.push_back(i);
                }

                if(spaceIndex.size() == 2)
                    break;
                if(spaceIndex.size() == 1 && rowLog.substr(0, spaceIndex[0]) == log_delStr)
                    break;
            }

            if(spaceIndex.size() == 1){
                operationType = rowLog.substr(0, spaceIndex[0]);
                std::stringstream keyStringStream(rowLog.substr(spaceIndex[0] + 1, spaceIndex[1] - spaceIndex[0] - 1));
                keyStringStream >> key;

                this->delKV(key);
            }
            else if(spaceIndex.size() == 2){
                operationType = rowLog.substr(0, spaceIndex[0]);

                std::stringstream keyStringStream(rowLog.substr(spaceIndex[0] + 1, spaceIndex[1] - spaceIndex[0] - 1));
                keyStringStream >> key;

                value = rowLog.substr(spaceIndex[1] + 1);

                this->putKV(key, value);
            }
        }
    }
}
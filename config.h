#pragma once

// SSTable: Header
#define sstable_headerSize 32

// SSTable: Bloom Filter
#define sstable_bfSize 8192

// SSTable: Key
#define sstable_keySize 8

// SSTable: Value
#define sstable_offsetSize 1024

// SSTable: Vlen
#define sstable_vlenSize 4

// SSTable: Limitation(16*1024)
#define sstable_maxSize 16384

// SSTable: Out of Range
#define sstable_out_of_range "~![ERROR] Out of Range!~"

// Log: Path
#define logFilePath "./WAL.log"

// MemTable: Delete Tag
#define delete_tag "~DELETED~"

// MemTable: Already Deleted
#define memtable_already_deleted "~![ERROR] ALREADY DELETED!~"

// MemTable: Not Found
#define memtable_not_exist "~![ERROR] MemTable No Exist!~"

// vLog: Exceed Limit
#define sstvalue_outOfRange "~![ERROR] Exceed Limit!~"

// vLog: No File to read
#define sstvalue_readFile_file "~![ERROR] No File!~"

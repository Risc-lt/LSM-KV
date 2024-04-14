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
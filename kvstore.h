#pragma once

#include "kvstore_api.h"
#include "sstable.h"
#include "memtable.h"
#include "vLog.h"
#include <sys/types.h>

class KVStore : public KVStoreAPI
{
	// You can add your implementation here
private:

	// Directory for storing sstables
	std::string SSTdir, vLogdir;
	// Limited number of sstable in each level
	std::map<uint64_t, std::string> levelLimit;
	// Index of sstable in each level
	std::map<uint64_t, std::map<uint64_t, SStable*> > levelIndex;
	
	/****************************************************
		levelIndex[level-i][timestamp] = sstable
	****************************************************/

	// Memtable
	MemTable* memtable;

	// vLog
	vLog* vlog;
	uint32_t curvLogOffset;

	// Maintain the current timestamp
	uint64_t sstMaxTimeStamp = 0;

	// Check all the files in the directory 
	void sstFileCheck(std::string path);

	// Compact the sstable in level i
	int mergeCheck();
	void merge(int level);

public:
	KVStore(const std::string &dir);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

	void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) override;

	void gc(uint64_t chunk_size) override;
};

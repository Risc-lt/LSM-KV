#pragma once

#include "kvstore_api.h"
#include "sstable.h"
#include "memtable.h"
#include <map>
#include <cstdint>


class KVStore : public KVStoreAPI
{
	// You can add your implementation here
private:
	// Path to the directory storing the sstable
	std::string dataDir;
	// Limit of each level
	std::map<uint64_t, uint64_t> levelLimit;
	// Type of the sstable: ssTable[lebel-i][timeStamp].sst
	std::map<uint64_t, std::map<uint64_t, SStable*>> ssTable;



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

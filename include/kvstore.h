#ifndef KVSTORE_H
#define KVSTORE_H

#include <string>
#include "leveldb/cache.h"
#include "leveldb/db.h"
#include <string>


class KVStore
{
public:
    leveldb::DB* db;
    KVStore();
    void store(std::string key, std::string value);
    std::string read(std::string key);
    void remove(std::string key);
    ~KVStore();
private:
};

#endif // KVSTORE_H
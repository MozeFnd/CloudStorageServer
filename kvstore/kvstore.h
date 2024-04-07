#ifndef KVSTORE_H
#define KVSTORE_H

#include <string>
#include "leveldb/cache.h"
#include "leveldb/db.h"
#include "common/util.h"
#include <string>

class KVStore
{
public:
    leveldb::DB* db;
    KVStore();
    void store(std::string key, std::string value);
    int append(std::string key, std::string value){
        std::string old = read(key);
        auto oldArr = splitStr(old, ',');
        for (auto str : oldArr) {
            if (value == str) {
                return 0;
            }
        }
        oldArr.push_back(value);
        store(key, mergeArr(oldArr, ','));
        return 1;
    }
    std::string read(std::string key);
    std::vector<std::string> readAsArray(std::string key) {
        std::string str = read(key);
        return splitStr(str, ',');
    }
    void remove(std::string key);
    ~KVStore();
private:
};

#endif // KVSTORE_H
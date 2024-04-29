#ifndef KVSTORE_H
#define KVSTORE_H

#include <string>
#include <vector>
#include <mutex>
#include "leveldb/cache.h"
#include "leveldb/db.h"
#include "common/util.h"
#include <string>

typedef std::unique_lock<std::mutex> Guard;

class LockManager{
public:
    LockManager(){}
    ~LockManager(){}
    std::mutex& get_lock(std::string key) {
        return locks[key];
    }

    void lock(std::string key) {
        Guard tmp_Guard(manager_lock);
        locks[key].lock();
    }

    void unlock(std::string key) {
        Guard tmp_Guard(manager_lock);
        locks[key].unlock();
    }
private:
    std::mutex manager_lock;
    std::unordered_map<std::string, std::mutex> locks;
};

class KVStore
{
public:
    leveldb::DB* db;
    std::mutex fetch_id_mut;
    KVStore();
    ~KVStore();
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

    // 取走一个空闲id
    uint32_t fetch_new_id() {
        std::unique_lock<std::mutex> tmp_lock(fetch_id_mut);
        auto key = "all_used_id";
        auto all_used_id_str = read(key);
        uint32_t new_id;
        if (all_used_id_str.size() == 0) {
            new_id = 1;
        } else {
            std::vector<std::string> all_used_id = splitStr(all_used_id_str, ',');
            uint32_t cur_max = std::stoul(all_used_id[all_used_id.size() - 1]);
            new_id = cur_max + 1;
        }
        all_used_id_str = all_used_id_str + "," + std::to_string(new_id);
        store(key, all_used_id_str);
        return new_id;
    }

    // 得到序列化成string的tree结构
    std::string read_tree_serialized(uint32_t id) {
        auto& tmp_lock = lock_manager.get_lock("tree_serialized");
        Guard tmp_guard(tmp_lock);
        auto key = "tree_" + std::to_string(id);
        auto val = read(key);
        return val;
    }

    // 将序列化成string的tree结构持久化
    void write_tree_serialiezed(uint32_t id, std::string serialiezed) {
        auto& tmp_lock = lock_manager.get_lock("tree_serialized");
        Guard tmp_guard(tmp_lock);
        auto key = "tree_" + std::to_string(id);
        store(key, serialiezed);
    }

private:
    LockManager lock_manager;
};

#endif // KVSTORE_H
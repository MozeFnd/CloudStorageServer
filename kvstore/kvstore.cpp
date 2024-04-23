#include "kvstore.h"

KVStore::KVStore() {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "testdb", &db);
}

std::string KVStore::read(std::string key){
    std::string value = "";
    leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &value);
    if (!s.ok()) {

    }
    return value;
}

void KVStore::store(std::string key, std::string value) {
    leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
    if (!s.ok()) {

    }
}

void KVStore::remove(std::string key){
    leveldb::Status s = db->Delete(leveldb::WriteOptions(), key);
    if (!s.ok()) {

    }
}

KVStore::~KVStore(){

}
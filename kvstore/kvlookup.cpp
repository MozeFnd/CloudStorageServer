#include "kvstore.h"
#include <iostream>
#include <string>
#include <memory>
#include <cassert>
using namespace std;

int main(int argc, char* argv[]) {
    shared_ptr<KVStore> kvstore = make_shared<KVStore>();
    assert(argc > 1);
    auto reqType = string(argv[1]);
    if (reqType == "store") {
        assert(argc == 4);
        string key = string(argv[2]); 
        string value = string(argv[3]); 
        kvstore->store(key, value);
        cout << "done" << endl;
    } else if (reqType == "read") {
        assert(argc == 3);
        string key = string(argv[2]); 
        cout << key << ": " << kvstore->read(key) << endl;
    } else {
        cout << "unknown type of request." << endl;
    }
    return 0;
}
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <memory>
#include <vector>
#include <fstream>
#include <filesystem>
#include "communicator/communicator.h"
#include "kvstore/kvstore.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "common/util.h"

constexpr int MAX_CLIENTS = 5;
constexpr int BUFFER_SIZE = 1024;



int main() {
    // std::string filepath = "storage/39/中文/empty.txt";
    // std::ifstream file(filepath, std::ios::binary);
    // if (!file.is_open()) {
    //     LOG_INFO("fail to open file {}", filepath);
    // }
    // auto buffer = (char*)malloc(100);
    // while (!file.eof()) {
    //     file.read(buffer , 10);
    //     int bytesRead = file.gcount();
    //     std::cout << "bytesRead: " << bytesRead << std::endl;
    // }
    // return 0;
    
    std::shared_ptr<KVStore> kvstore = std::make_shared<KVStore>();
    std::shared_ptr<Communicator> comm = std::make_shared<Communicator>(kvstore);
    std::vector<std::thread> pool;
    std::thread loop(&Communicator::loop, comm, std::ref(pool));
    std::cout << "listening loop starts..." << std::endl;
    // for (auto& t : pool) {
    //     t.join();
    // }
    loop.join();
    return 0;
}
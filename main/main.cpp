#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <memory>
#include "communicator/communicator.h"
#include "kvstore/kvstore.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

constexpr int MAX_CLIENTS = 5;
constexpr int BUFFER_SIZE = 1024;



int main() {
    std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt("file_logger", "logfile.txt");
    logger->set_level(spdlog::level::debug);

    std::cout << "hello world" << std::endl;
    return 0;

    // logger->info("sys start1");
    // logger->debug("sys start2");
    // logger->flush();
    
    std::shared_ptr<KVStore> kvstore = std::make_shared<KVStore>();
    std::shared_ptr<Communicator> comm = std::make_shared<Communicator>(logger, kvstore);
    
    comm->loop();
    return 0;
}
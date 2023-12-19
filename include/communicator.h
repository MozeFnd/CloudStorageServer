#ifndef COMMUNICATOR_H_
#define COMMUNICATOR_H_
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <memory>
#include "kvstore.h"
#include "spdlog/spdlog.h"
#include "jsontool.h"

class Communicator {
public:
    enum RequestType {
        Login = 1,
        AcquireID = 2,
        GetAllDir = 3,
        AddDir = 4,
    };
private:
    int serverSocket;
    struct sockaddr_in serverAddr;
    const uint16_t port = 6900;
    const int MAX_CLIENTS = 10;
    const int BUFFER_SIZE = 1024;
    std::shared_ptr<KVStore> kvstore;
    std::shared_ptr<spdlog::logger> logger;
public:
    Communicator(std::shared_ptr<spdlog::logger> logger, std::shared_ptr<KVStore> kvstore);
    ~Communicator();
    void loop();
    void handleRequests(int socket_fd);
    void handleLogin(int clientSocket);
    void handleAcquireID(int clientSocket);
    void handleAddNewDirectory(int clientSocket);
    void handleGetAllDir(int clientSocket);
};


#endif
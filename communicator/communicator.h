#ifndef COMMUNICATOR_H_
#define COMMUNICATOR_H_
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <memory>
#include "kvstore/kvstore.h"
#include "spdlog/spdlog.h"
#include "jsontool/jsontool.h"
#include "common/tree.h"

class Communicator {
public:
    enum RequestType {
        Login = 1,
        AcquireID = 2,
        GetAllDir = 3,
        AddDir = 4,
        SyncFile = 5,
        GetRemoteTree = 6,
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
    Communicator(std::shared_ptr<KVStore> kvstore);
    ~Communicator();
    void loop();
    void handleRequests(int socket_fd);
    void handleLogin(int clientSocket);
    void handleAcquireID(int clientSocket);
    void handleAddNewDirectory(int clientSocket);
    void handleGetAllDir(int clientSocket);
    void handleSyncFile(int clientSocket);
    void handleGetRemoteTree(int clientSocket);
    void recordDirectoryInfo(std::string id, std::shared_ptr<Json> js);
    std::unordered_map<std::string, std::shared_ptr<Json>> readAllDirectoryInfo();
};


#endif
#ifndef COMMUNICATOR_H_
#define COMMUNICATOR_H_
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <string>
#include <memory>
#include <vector>
#include "kvstore/kvstore.h"
#include "spdlog/spdlog.h"
#include "jsontool/jsontool.h"
#include "common/tree.h"

class Communicator {
public:
    enum RequestType{
        EXIT = 99,
        Login = 1,
        AcquireID = 2,
        GetAllDir = 3,
        AddDir = 4,
        UploadFile = 5,
        DownloadFile = 6,
        GetRemoteTree = 7,
        UpdateRemoteTree = 8,
    };
private:
    int serverSocket;
    std::mutex pool_mut;

    struct sockaddr_in serverAddr;
    const uint16_t port = 6900;
    const int MAX_CLIENTS = 10;
    const int BUFFER_SIZE = 1024;
    std::shared_ptr<KVStore> kvstore;
    std::shared_ptr<spdlog::logger> logger;
public:
    Communicator(std::shared_ptr<KVStore> kvstore);
    ~Communicator();
    void loop(std::vector<std::thread>& pool);
    void handleRequests(int socket_fd);
    void handleLogin(int clientSocket);
    void handleAcquireID(int clientSocket);
    void handleAddNewDirectory(int clientSocket);
    void handleGetAllDir(int clientSocket);
    void handleUploadFile(int clientSocket);
    void handleDownloadFile(int clientSocket);
    void handleGetRemoteTree(int clientSocket);
    void recordDirectoryInfo(std::string id, std::shared_ptr<Json> js);
    std::unordered_map<std::string, std::shared_ptr<Json>> readAllDirectoryInfo();
};


#endif
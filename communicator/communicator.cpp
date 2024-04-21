#include "communicator.h"
#include <functional>
#include <filesystem>
#include <fstream>

Communicator::Communicator(std::shared_ptr<spdlog::logger> logger, std::shared_ptr<KVStore> kvstore) {
    this->logger = logger;
    this->kvstore = kvstore;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    int local = bind(serverSocket, (sockaddr*)(&serverAddr), sizeof(serverAddr));
    std::cout << "local: " << local << std::endl;
    if (local == -1) {
        std::cerr << "Bind failed\n";
        close(serverSocket);
        return;
    }

    int res = listen(serverSocket, MAX_CLIENTS);
    if (res == -1) {
        std::cerr << "Listen failed\n";
        close(serverSocket);
        return;
    }

    std::cout << "Server listening on port 6900...\n";
}

Communicator::~Communicator() {
}

void Communicator::handleRequests(int socket_fd) {
    int BUFFER_SIZE = 100;
    int clientSocket = socket_fd;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    
    int bytesReceived = recv(clientSocket, buffer, 1, 0);
    uint8_t requestType = buffer[0];
    logger->info("requestType: {}", requestType);
    if (requestType == Login) {
        handleLogin(clientSocket);
    } else if (requestType == AcquireID) {
        handleAcquireID(clientSocket);
    } else if (requestType == AddDir) {
        handleAddNewDirectory(clientSocket);
    } else if (requestType == SyncFile) {
        handleSyncFile(clientSocket);
    } else if (requestType == GetAllDir) {

    } else {
        logger->error("unknown request type: {}", requestType);
    }
    logger->flush();
    // std::cout << bytesReceived << "bytes received: ";
    // for (int i = 0;i < bytesReceived;i++) {
    //     std::cout << (uint32_t)buffer[i] << " ";
    // }
    // std::cout << std::endl;
    send(clientSocket, buffer, bytesReceived, 0);
    memset(buffer, 0, sizeof(buffer));

    close(clientSocket);
}

void blockRead(int clientSocket, char* buffer, int n) {
    int acc = 0;
    while (acc < n) {
        acc += recv(clientSocket, buffer + acc, n - acc, 0);
    }
    return;
}

std::string receiveStringWithLen(int clientSocket) {
    int buffer_size = 1000;
    char* buffer = (char*)malloc(buffer_size);
    recv(clientSocket, buffer, 1, 0);
    uint8_t len = buffer[0];
    memset(buffer, 0, 1);

    recv(clientSocket, buffer, len, 0);
    buffer[len] = '\0';
    std::string str(buffer);
    delete buffer;
    return str;
}

void Communicator::handleLogin(int clientSocket) {
    std::string uname = receiveStringWithLen(clientSocket);
    std::string pwd = receiveStringWithLen(clientSocket);
    logger->info("client login in: uname:{}   pwd:{}", uname, pwd);
}

void Communicator::handleAcquireID(int clientSocket) {
    int buffer_size = 1000;
    char* buffer = (char*)malloc(buffer_size);
    std::string nextID = kvstore->read("nextVacantID");
    if (nextID.empty()) {
        nextID = "1";
    }
    uint32_t startID = std::stoul(nextID);
    int batchSize = 10;
    *buffer = (uint8_t)batchSize;
    for (int i = 0;i < batchSize;i++) {
        *(uint32_t*)(buffer + 1 + i * 4) = i + startID;
    }
    kvstore->store("nextVacantID", std::to_string(batchSize + startID));
    logger->info("send id from {} to {}", startID, startID + batchSize - 1);
    send(clientSocket, buffer, 1 + batchSize * 4, 0);
    delete buffer;
}

void Communicator::handleAddNewDirectory(int clientSocket) {
    int buffer_size = 1000;
    char* buffer = (char*)malloc(buffer_size);
    blockRead(clientSocket, buffer, 4);
    uint32_t len = *(uint32_t*)buffer;
    memset(buffer, 0, 4);
    blockRead(clientSocket, buffer, len);
    buffer[len] = '\0';
    std::string jsonStr(buffer);
    auto js = Json::fromJsonString(jsonStr);
    auto dev_name = js->getProperty("device");
    auto id = js->getProperty("id");
    auto dir_name = js->getProperty("name");
    auto archJs = js->getChildren("arch")[0];
    
    std::string storage_path = "../storage/" + id;
    std::filesystem::create_directory(storage_path);

    std::function<void(std::shared_ptr<Json>, std::string)> mkdir;
    mkdir = [&mkdir](std::shared_ptr<Json> cur_js, std::string parentPath){
        if ("directory" == cur_js->getProperty("type")) {
            auto name = cur_js->getProperty("name");
            auto to_create = parentPath + "/" + name;
            std::filesystem::create_directory(to_create);
            for (auto child : cur_js->getChildren("contents")) {
                mkdir(child, to_create);
            }
        }
    };

    mkdir(archJs, storage_path);
    recordDirectoryInfo(id, archJs);

    logger->info("device {} add new directory to track, id: {}, name: {}", dev_name, id, dir_name);
    free(buffer);
}

// ┌───────────────┬─────────────────────────┐
// │SyncFile(1byte)│    json_size(4bytes)    │
// ├───────────────┴─────────────────────────┤
// │          json(id,relative_path)         │
// └─────────────────────────────────────────┘
// ┌────────────────────────────────┬───────────────┐
// │        data_size(4bytes)       │ is_last(1byte)│
// ├────────────────────────────────┴───────────────┤ 
// │          data(1000bytes at most)               │
// │                                                │
// └────────────────────────────────────────────────┘
void Communicator::handleSyncFile(int clientSocket){
    int buffer_size = 20 + 3e4;
    char* buffer = (char*)malloc(buffer_size);
    blockRead(clientSocket, buffer, 4);
    uint32_t len = *(uint32_t*)buffer;
    memset(buffer, 0, 4);
    blockRead(clientSocket, buffer, len);
    buffer[len] = '\0';
    std::string jsonStr(buffer);
    auto js = Json::fromJsonString(jsonStr);
    
    auto id = js->getProperty("id");
    auto relativePath = js->getProperty("relative_path");
    auto filepath = "../storage/" + id + "/" + relativePath;
    std::cout << "filepath:" <<filepath << std::endl;
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        logger->error("fail to open/create file {}", filepath);
    }
    int cnt = 0;
    while (true) {
        cnt++;
        if (cnt % 50 == 0) {
            logger->info("cnt = {}", cnt);
            logger->flush();
        }
        blockRead(clientSocket, buffer, 4);
        auto data_size = *(uint32_t*)buffer;
        blockRead(clientSocket, buffer, 1);
        bool is_last = (*buffer == 1);
        blockRead(clientSocket, buffer, data_size);
        file.write(buffer, data_size);
        if (is_last) {
            break;
        }
    }
    file.close();

    logger->info("archive (id={}) updates file {}, cnt = {}", id, relativePath, cnt);
    logger->flush();

    free(buffer);
}

void handleGetRemoteTree(int clientSocket) {
    
}

void handleGetAllDir(int clientSocket){
    Json js;
    js.addProperty("dir_num", "10");

}

void Communicator::recordDirectoryInfo(std::string id, std::shared_ptr<Json> js) {
    kvstore->store("dir_" + id, js->toString());
    kvstore->append("all_dir_ids", id);
}

std::unordered_map<std::string, std::shared_ptr<Json>> Communicator::readAllDirectoryInfo(){
    auto allDirIds = Json::fromJsonString(kvstore->read("all_dir_ids"));
    auto ids = kvstore->readAsArray("all_dir_ids");
    std::unordered_map<std::string, std::shared_ptr<Json>> ret;
    for (auto id : ids) {
        auto jsonStr = kvstore->read("dir_" + id);
        ret[id] = Json::fromJsonString(jsonStr);
    }
    return ret;
}

void Communicator::loop() {
    int clientSocket;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    // while (true) {
    //     clientSocket = accept(serverSocket, (sockaddr*)(&clientAddr), &clientAddrSize);
    //     if (clientSocket == -1) {
    //         std::cerr << "Accept failed\n";
    //         close(serverSocket);
    //         return;
    //     }
    //     std::cout << "connection built, clientSocket: " << clientSocket << std::endl;
    //     std::thread t = std::thread(&Communicator::handleRequests, this, clientSocket);
    //     t.detach();
    // }

    clientSocket = accept(serverSocket, (sockaddr*)(&clientAddr), &clientAddrSize);
    std::cout << "connection built, clientSocket: " << clientSocket << std::endl;
    handleRequests(clientSocket);
}
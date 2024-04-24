#include "communicator.h"
#include <functional>
#include <filesystem>
#include <fstream>

Communicator::Communicator(std::shared_ptr<KVStore> kvstore) {
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
    LOG_INFO("requestType: " + std::to_string(requestType));
    if (requestType == Login) {
        handleLogin(clientSocket);
    } else if (requestType == AcquireID) {
        handleAcquireID(clientSocket);
    } else if (requestType == AddDir) {
        handleAddNewDirectory(clientSocket);
    } else if (requestType == SyncFile) {
        handleSyncFile(clientSocket);
    } else if (requestType == GetAllDir) {

    } else if (requestType == GetRemoteTree) {
        handleGetRemoteTree(clientSocket);
    } else {
        LOG_INFO("ERROR: unknown request type");
    }
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

void blockWrite(int clientSocket, char* buffer, int n) {
    int acc = 0;
    while (acc < n) {
        acc += send(clientSocket, buffer + acc, n - acc, 0);
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
    free(buffer);
    return str;
}

void Communicator::handleLogin(int clientSocket) {
    std::string uname = receiveStringWithLen(clientSocket);
    std::string pwd = receiveStringWithLen(clientSocket);
    // logger->info("client login in: uname:{}   pwd:{}", uname, pwd);
}

void Communicator::handleAcquireID(int clientSocket) {
    int buffer_size = 10;
    char* buffer = (char*)malloc(buffer_size);
    
    uint32_t nextID = kvstore->fetch_new_id();
    // logger->info("send id {}", nextID);
    *(uint32_t*)buffer = nextID;
    blockWrite(clientSocket, buffer, 4);
    free(buffer);
}

void Communicator::handleAddNewDirectory(int clientSocket) {
    int buffer_size = 10;
    char* buffer = (char*)malloc(buffer_size);
    blockRead(clientSocket, buffer, 4);
    uint32_t len = *(uint32_t*)buffer;
    free(buffer);
    buffer_size = len;
    buffer = (char*)malloc(buffer_size);
    std::cout << "len: " << len << std::endl;

    blockRead(clientSocket, buffer, len);

    std::string serialized(buffer, len);
    
    std::shared_ptr<Node> root = Node::fromSerializedStr(serialized);

    std::string storage_path = "storage/" + root->id;
    
    std::string print = "structure:\n";
    Node::formatted(root, print, 0);
    LOG_INFO(print);
    // old version
    // std::function<void(std::shared_ptr<Json>, std::string)> mkdir;
    // mkdir = [&mkdir](std::shared_ptr<Json> cur_js, std::string parentPath){
    //     if ("directory" == cur_js->getProperty("type")) {
    //         auto name = cur_js->getProperty("name");
    //         auto to_create = parentPath + "/" + name;
    //         std::filesystem::create_directory(to_create);
    //         for (auto child : cur_js->getChildren("contents")) {
    //             mkdir(child, to_create);
    //         }
    //     }
    // };

    std::function<void(std::shared_ptr<Node>, std::string)> mkdir;
    mkdir = [&mkdir](std::shared_ptr<Node> cur_node, std::string parentPath){
        if (DIRECTORY == cur_node->file_type) {
            auto name = cur_node->name;
            auto to_create = parentPath + "/" + name;
            LOG_INFO(to_create);
            std::filesystem::create_directories(to_create);
            for (auto child : cur_node->children) {
                mkdir(child, to_create);
            }
        }
    };

    mkdir(root, storage_path);

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
        // logger->error("fail to open/create file {}", filepath);
    }
    int cnt = 0;
    while (true) {
        cnt++;
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

    // logger->info("archive (id={}) updates file {}, cnt = {}", id, relativePath, cnt);
    // logger->flush();

    free(buffer);
}

void Communicator::handleGetRemoteTree(int clientSocket) {
    // receive and lookup
    int buffer_size = 10;
    char* buffer = (char*)malloc(buffer_size);
    blockRead(clientSocket, buffer, 4);
    uint32_t root_id = *(uint32_t*)buffer;
    std::string serialized = kvstore->read_tree_serialized(root_id);
    free(buffer);

    // send
    buffer_size = serialized.size();
    buffer = (char*)malloc(buffer_size);
    memset(buffer, 0, buffer_size);
    const char* tmp = serialized.c_str();
    memcpy(buffer, tmp, buffer_size);
    blockWrite(clientSocket, buffer, buffer_size);
    free(buffer);
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
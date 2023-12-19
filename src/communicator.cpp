#include "communicator.h"

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
    recv(clientSocket, buffer, 4, 0);
    uint32_t len = *(uint32_t*)buffer;
    memset(buffer, 0, 4);
    recv(clientSocket, buffer, len, 0);
    buffer[len] = '\0';
    std::string jsonStr(buffer);
    auto js = Json::fromJsonString(jsonStr);
    auto dev_name = js->getProperty("device");
    auto id = js->getProperty("id");
    auto dir_name = js->getProperty("name");
    kvstore->store("dir_" + id + "_name", dir_name);
    auto allJsonStr = kvstore->read("all_dir_json");
    auto allJs = Json::fromJsonString(allJsonStr);
    allJs->addChild("dirs", js);
    auto num = allJs->getProperty("num");
    if (num.empty()) {
        allJs->addProperty("num", "1");
    } else {
        allJs->addProperty("num", std::to_string(std::stoul(num) + 1));
    }
    kvstore->store("all_dir_json", allJs->toString());
}

void handleGetAllDir(int clientSocket){
    Json js;
    js.addProperty("dir_num", "10");
}

void Communicator::loop() {
    int clientSocket;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    while (true) {
        clientSocket = accept(serverSocket, (sockaddr*)(&clientAddr), &clientAddrSize);
        if (clientSocket == -1) {
            std::cerr << "Accept failed\n";
            close(serverSocket);
            return;
        }
        std::cout << "connection built, clientSocket: " << clientSocket << std::endl;
        std::thread t = std::thread(&Communicator::handleRequests, this, clientSocket);
        t.detach();
    }

    // clientSocket = accept(serverSocket, (sockaddr*)(&clientAddr), &clientAddrSize);
    // handleRequests(clientSocket);
}
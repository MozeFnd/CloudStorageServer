#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <memory>
#include <vector>
#include <filesystem>
#include "communicator/communicator.h"
#include "kvstore/kvstore.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "common/util.h"

constexpr int MAX_CLIENTS = 5;
constexpr int BUFFER_SIZE = 1024;



int main() {

    // std::cout << "hello world" << std::endl;
    // std::string str = "C:/Users/10560/Desktop/驾照";
    // std::wstring wstr = str2wstr(str);
    // std::string tmp = wstr2str(wstr);
    // std::cout << tmp << std::endl;
    // std::cout << "str.size(): " << str.size() << std::endl;
    // std::cout << "<< sizeof(wchar_t): " << sizeof(wchar_t) << std::endl;
    // std::locale::global(std::locale("en_US.UTF-8"));
    // std::wstring wstr = L"驾照/A/B";
    // for (wchar_t wch : wstr) {
    //     WCharUnion wu;
    //     wu.wc = wch;
    //     std::cout << std::hex << (int)wu.bytes.byte1 << " " << std::hex << (int)wu.bytes.byte2 << " " 
    //     << std::hex << (int)wu.bytes.byte3 << " "<< std::hex << (int)wu.bytes.byte4 << std::endl;
    // }
    // std::wcout << wstr << std::endl;
    // std::filesystem::path p(wstr);
    // std::filesystem::create_directories(p);
    // std::string str = "\xE9\xA9\xBE\xE7\x85\xA7";
    // std::filesystem::create_directories(str+"/A/B");
    // std::wstring tmp = str2wstr(str);
    // std::cout << str << std::endl;
    // std::wcout << tmp << std::endl;
    // return 0;
    // std::cout << "str.size(): " << str.size() << std::endl; 
    // for (auto ch : str) {
    //     std::cout << std::hex << (int8_t)ch << " ";
    // }
    // std::cout << std::endl;
    // return 0;
    // std::wstring wstr = str2wstr(str);
    // std::string new_str = wstr2str(wstr);
    // std::cout << str << " " << new_str << std::endl;
    // std::filesystem::create_directories(new_str+"/A/B");
    // return 0;
    
    // auto tmp_fun = [&](int num){
    //     LOG_INFO("num: {}", num);
    // };
    // std::vector<std::thread> pool;
    // for (size_t i = 0;i < 10;i++) {
    //     // std::thread t = std::thread(tmp_fun, i);
    //     pool.emplace_back(std::thread(tmp_fun, i));
    // }
    // for (size_t i = 0;i < 10;i++) {
    //     pool[i].join();
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
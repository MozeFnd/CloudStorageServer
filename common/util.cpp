#include "util.h"

std::shared_ptr<spdlog::logger> logger = nullptr;

// template <typename... Args>
// void LOG_INFO(const std::string& text, Args &&...args) {
//     if (logger == nullptr) {
//         logger = spdlog::basic_logger_mt("file_logger", "logfile.txt");
//     }
//     std::cout << "try to call LOG_INFO, thread id: " << std::this_thread::get_id() << std::endl;
//     logger->set_level(spdlog::level::debug);
//     logger->info(text, std::forward<Args>(args)...);
//     logger->flush();
// }

#ifndef LOGGER_H_
#define LOGGER_H_
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

class Logger
{
private:
    std::shared_ptr<spdlog::logger> logger;
public:
    Logger();
    ~Logger();
};

Logger::Logger() {
    logger = spdlog::basic_logger_mt("file_logger", "logfile.txt");
}



Logger::~Logger() {
}


#endif
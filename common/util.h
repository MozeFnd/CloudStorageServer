#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#define DEBUG_MOD

union WCharUnion {
    wchar_t wc;  // 宽字符
#ifdef __linux__
    struct {
        uint8_t byte1;  // 第1个字节
        uint8_t byte2;  // 第2个字节
        uint8_t byte3;  // 第3个字节
        uint8_t byte4;  // 第4个字节
    } bytes;
#else
    struct {
        uint8_t byte1;  // 第1个字节
        uint8_t byte2;  // 第2个字节
    } bytes;
#endif

};

struct FILETIME {
    uint32_t dwLowDateTime;
    uint32_t dwHighDateTime;
};

// inline std::wstring str2wstr(std::string str){
//     std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
//     std::wstring wstr = converter.from_bytes(str);
//     return wstr;
// }

// inline std::string wstr2str(const std::wstring wstr)
// {
//     std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
//     std::string utf8String = conv.to_bytes(wstr);
//     return utf8String;
// }

inline std::wstring str2wstr(std::string str){
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(str);
}

inline std::string wstr2str(const std::wstring wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}


inline std::vector<std::string> splitStr(std::string str, char delimiter) {
    std::vector<std::string> arr;
    std::string cur = str;
    while (true) {
        size_t slash_pos = cur.find(delimiter);
        if (slash_pos == std::string::npos) {
            arr.push_back(cur);
            break;
        } else {
            std::string tmp = cur.substr(0, slash_pos);
            arr.push_back(tmp);
            cur = cur.substr(slash_pos+1);
        }
    }
    return arr;
}

inline std::string mergeArr(std::vector<std::string> arr, char delimiter) {
    if (arr.size() == 0) {
        return "";
    }
    std::string ret = arr[0];
    for (size_t i = 1;i < arr.size();i++) {
        ret += delimiter + arr[i];
    }
    return ret;
}

inline FILETIME MicrosecondsToFileTime(uint64_t microseconds) {
    uint64_t tmp;
    tmp = (microseconds + 11644473600ULL * 1000000ULL) * 10;  // Convert microseconds to 100-nanosecond intervals and adjust for the FILETIME epoch
    FILETIME filetime;
    filetime.dwLowDateTime = tmp & 0xFFFFFFFF;
    filetime.dwHighDateTime = static_cast<uint32_t>(tmp >> 32);
    return filetime;
}

inline uint64_t FileTimeToMicroseconds(const FILETIME& filetime) {
    uint64_t tmp;
    tmp = filetime.dwLowDateTime;
    tmp |= (static_cast<uint64_t>(filetime.dwHighDateTime) << 32);
    return tmp / 10 - 11644473600000000ULL;  // Convert 100-nanosecond intervals to microseconds and adjust for the Unix epoch
}

inline uint64_t max_filetime(FILETIME t1, FILETIME t2) {
    uint64_t stamp1 = FileTimeToMicroseconds(t1);
    uint64_t stamp2 = FileTimeToMicroseconds(t2);
    if (stamp1 < stamp2) {
        return stamp2;
    }
    return stamp1;
}

template <typename... Args>
inline void LOG_INFO(const std::string& text, Args &&...args) {
    static std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt("file_logger", "logfile.txt");
    logger->set_level(spdlog::level::debug);
    logger->info(text, std::forward<Args>(args)...);
    logger->flush();
}

#endif // UTIL_H
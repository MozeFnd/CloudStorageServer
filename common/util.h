#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <vector>

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
    for (int i = 1;i < arr.size();i++) {
        ret += delimiter + arr[i];
    }
    return ret;
}

#endif // UTIL_H
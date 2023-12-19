#include "jsontool.h"
#include <filesystem>
#include <stack>
#include <regex>

#ifndef QT_ENVIRONMENT
#include <iostream>
#endif

Json::Json(){

}

void Json::addProperty(std::string name, std::string value){
    properties_[name] = value;
}

void Json::addChild(std::string name, std::shared_ptr<Json> child){
    children_[name].push_back(child);
}

std::string Json::toString(){
    std::string ret;
    ret.append("{");
    size_t cnt = 0;
    for (auto& [name, value] : properties_) {
        if (cnt != 0) {
            ret.append(",");
        }
        ret.append("\"" + name + "\"");
        ret.append(":");
        ret.append("\"" + value + "\"");
        // if (values.size() == 1) {
        //     ret.append("\"" + values[0] + "\"");
        // } else {
        //     ret.append("[");
        //     bool is_first = true;
        //     for (auto value : values) {
        //         if (is_first) {
        //             is_first = false;
        //             ret.append("\"" + values[0] + "\"");
        //         } else {
        //             ret.append(",\"" + values[0] + "\"");
        //         }
        //     }
        //     ret = ret.substr(0, ret.size() - 1);
        //     ret.append("]");
        // }
        cnt++;
    }

    for (auto [name, children] : children_) {
        if (cnt != 0) {
            ret.append(",");
        }
        ret.append("\"" + name + "\":[");
        bool is_first = true;
        for (auto child : children) {
            if (is_first) {
                is_first = false;
                ret.append(child->toString());
            } else {
                ret.append("," + child->toString());
            }
        }
        ret.append("]");
    }
    ret.append("}");
    return ret;
}

std::shared_ptr<Json> Json::fromPath(std::string path){
    std::shared_ptr<Json> ret = std::make_shared<Json>();
    ret->addProperty("type", "direcory");
    std::filesystem::path p = path;
    auto str = p.filename().string();
    ret->addProperty("dirname", str);
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            std::shared_ptr<Json> tmp = std::make_shared<Json>();
            tmp->addProperty("type", "regular_file");
            tmp->addProperty("filename", entry.path().filename().string());
            tmp->addProperty("size", std::to_string(std::filesystem::file_size(entry.path())));
            ret->addChild("contents", tmp);
        } else if (entry.is_directory()) {
            auto toSearch = entry.path().string();
            ret->addChild("contents", fromPath(toSearch));
        }
    }
    return ret;
}

std::shared_ptr<Json> Json::fromJsonString(std::string str) {
    std::stack<uint32_t> left_brackts;
    std::unordered_map<uint32_t, uint32_t> right_brackts;
    for (size_t i = 0;i < str.size();i++) {
        if (str[i] == '{' || str[i] == '[') {
            left_brackts.push(i);
        }
        else if (str[i] == '}' || str[i] == ']') {
            right_brackts[left_brackts.top()] = i;
            left_brackts.pop();
        }
    }
    std::shared_ptr<Json> js = std::make_shared<Json>();

    std::regex patternA("\"([^\"]*)\":\"([^\"]*)\"");
    std::regex patternB("\"([^\"]*)\":\\[");
    std::string strCopy = str;
    while (true) {
        std::smatch matches;
        if(!std::regex_search(strCopy, matches, patternA) || matches.position() > 1) {
            if (!std::regex_search(str, matches, patternB)) {
                break;
            }
            auto left_square = matches.position() + matches.length() - 1;
            auto right_square = right_brackts[left_square];
            auto left_curly = left_square + 1;

            while (left_curly < right_square) {
                auto right_curly = right_brackts[left_curly];
                auto tmp_child = fromJsonString(str.substr(left_curly, right_curly - left_curly + 1));
                js->addChild("contents", tmp_child);
                left_curly = right_curly + 2;
            }
            break;
        }
        js->addProperty(matches[1].str(), matches[2].str());
        strCopy = matches.suffix().str();
    }
    return js;
}

void Json::formattedOutput(std::string jsonStr) {
    std::unordered_map<uint32_t, std::string> spaces;
    spaces[0] = "";
    for (int i = 1;i < 20;i++) {
        spaces[i] = spaces[i - 1] + "  ";
    }
    int left = 0;
    std::string ret = "";
    for (int i = 0;i < jsonStr.size();i++) {
        char ch = jsonStr[i];
        if (ch == '{' || ch == '[') {
            left++;
            auto toAppend = ch + ('\n' + spaces[left]);   // 打印后换行
            ret.append(toAppend);
        } else if (ch == '}' || ch == ']') {
            left--;
            ret.append("\n" + spaces[left]);  //打印前换行
            ret += ch;
        } else if (ch == ',') {
            ret.append(",\n" + spaces[left]);  // 打印后换行
        } else {
            ret += ch;
        }
    }
#ifdef QT_ENVIRONMENT
    while (!ret.empty()) {
        auto pos = ret.find('\n');
        if (pos != std::string::npos) {
            auto toPrint = ret.substr(0, pos);
            qDebug() << QString::fromStdString(toPrint);
            ret = ret.substr(pos + 1);
        } else {
            qDebug() << QString::fromStdString(ret);
            break;
        }
    }
#else
    std::cout << ret << std::endl;
#endif

    return;
}
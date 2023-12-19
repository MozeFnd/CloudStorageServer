#ifndef JSONTOOL_H
#define JSONTOOL_H

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

// #define QT_ENVIRONMENT

class Json {
public:
    Json();
    void addProperty(std::string name, std::string value);
    void addChild(std::string name, std::shared_ptr<Json> child);
    std::string toString();
    static std::shared_ptr<Json> fromPath(std::string path);
    static std::shared_ptr<Json> fromJsonString(std::string str);
    static void formattedOutput(std::string jsonStr);
    std::string getProperty(std::string key) {
        if (properties_[key].size() == 0) {
            return "";
        }
        return properties_[key];
    }
    std::vector<std::shared_ptr<Json>> getChildren(std::string key) {
        return children_[key];
    }
private:
    std::unordered_map<std::string, std::string> properties_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Json>>> children_;
};


#endif // JSONTOOL_H

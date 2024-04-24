#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <queue>
#include "common/util.h"
#include "protobuf/node.pb.h"

#define DIRECTORY 0
#define REGULAR_FILE 1

class Node{
public:
    bool is_root;
    uint32_t file_type;
    uint32_t id;     // 每个资料库都有一个全局唯一ID，一个资料库的所有节点公用同一ID

#ifdef __linux__
    std::string abs_path;   // 绝对路径
    std::string relative_path;     // 相对路径
    std::string name; // 文件名 或 文件夹名
    std::string init_str = "using linux";
#else
    std::wstring abs_path;   // 绝对路径
    std::wstring relative_path;     // 相对路径
    std::wstring name; // 文件名 或 文件夹名
    std::string init_str = "using windows";
#endif

    std::string md5;   // md5码

    FILETIME last_modified;   //上次修改的时间
    FILETIME max_last_modified;  // 所有子节点中last_modified的最大值
    std::vector<std::shared_ptr<Node>> children;

public:
    Node() : children{} {
        std::cout << init_str << std::endl;
        id = 0;
    }

    void toPbNode(tree::Node* pb_node);

    std::string serialize();

    void add_child(std::shared_ptr<Node> child) { children.push_back(child); }

    static std::shared_ptr<Node> fromPbNode(tree::Node* pb_node);

    static std::shared_ptr<Node> fromSerializedStr(const std::string& serialezed);

    // static std::shared_ptr<Node> fromPath(std::wstring abs_path, std::wstring relative_path, bool is_root);
    static void formatted(std::shared_ptr<Node> cur_node, std::string& cur_str, uint32_t layer) {
        std::string indent = "";
        for (size_t i = 0;i < layer;i++) {
            indent += "  ";
        }
#ifdef __linux__
        cur_str = cur_str + indent + cur_node->name + "\n";
#else
        cur_str = cur_str + indent + wstr2str(cur_node->name) + "\n";
#endif
        for (auto child : cur_node->children) {
            formatted(child, cur_str, layer + 1);
        }
    }
};

#endif // TREE_H

#include "tree.h"
#include <filesystem>

std::shared_ptr<Node> Node::fromPbNode(tree::Node* pb_node) {
    auto node = std::make_shared<Node>();
    node->is_root = pb_node->is_root();
    node->file_type = pb_node->file_type();
    node->id = pb_node->id();
#ifdef __linux__
    node->abs_path = pb_node->abs_path();
    node->relative_path = pb_node->relative_path();
    node->name = pb_node->name();
#else
    node->abs_path = str2wstr(pb_node->abs_path());
    node->relative_path = str2wstr(pb_node->relative_path());
    node->name = str2wstr(pb_node->name());
#endif
    node->last_modified = MicrosecondsToFileTime(pb_node->last_modified_tmstmp());
    node->max_last_modified = MicrosecondsToFileTime(pb_node->max_last_modified_tmstmp());

    for (auto pb_child : pb_node->children()) {
        auto child = fromPbNode(&pb_child);
        node->add_child(child);
        continue;
    }
    return node;
}

void Node::toPbNode(tree::Node* pb_node) {
    pb_node->set_is_root(is_root);
    pb_node->set_file_type(file_type);
    pb_node->set_id(id);
#ifdef __linux__
    pb_node->set_abs_path(abs_path);
    pb_node->set_relative_path(relative_path);
    pb_node->set_name(name);
#else
    pb_node->set_abs_path(wstr2str(abs_path));
    pb_node->set_relative_path(wstr2str(relative_path));
    pb_node->set_name(wstr2str(name));
#endif
    pb_node->set_last_modified_tmstmp(FileTimeToMicroseconds(last_modified));
    pb_node->set_max_last_modified_tmstmp(FileTimeToMicroseconds(max_last_modified));

    for (auto child : children) {
        auto pb_child = pb_node->add_children();
        child->toPbNode(pb_child);
    }
}

std::string Node::serialize() {
    tree::Node* pb_node = new tree::Node();
    toPbNode(pb_node);
    std::string ret = pb_node->SerializeAsString();
    delete pb_node;
    return ret;
}

std::shared_ptr<Node> Node::fromSerializedStr(const std::string& serialized) {
    auto pb_node = new tree::Node();
    pb_node->ParseFromString(serialized);
    LOG_INFO("pb_node->name(): "+ pb_node->name());
    auto ret = fromPbNode(pb_node);
    delete pb_node;
    return ret;
}

std::shared_ptr<Node> Node::findChildByRelativePath(std::shared_ptr<Node> cur_node, std::string relative_path) {
    auto vec = splitStr(relative_path, '/');
    if (vec.size() == 0) {
        std::cerr << "empty relative_path" << std::endl;
    }
    std::string name_str = cur_node->name;
    if (vec[0] != name_str) {
        std::cerr << "vec[0]: " << vec[0] << "  name: " << name_str << std::endl;
    }
    if (vec.size() == 1) {
        return cur_node;
    }
    for (auto child : cur_node->children) {
        if (child->name == vec[1]) {
            std::string next = relative_path.substr(vec[0].size() + 1);
            return findChildByRelativePath(child, next);
        }
    }
    return nullptr;
}

void Node::updateTimestamp(std::shared_ptr<Node> cur_node, std::string relative_path, uint64_t timestamp) {
    auto vec = splitStr(relative_path, '/');
    std::string name_str = cur_node->name;
    if (vec.size() == 0 || vec[0] != name_str) {
        std::cerr << "empty relative_path" << std::endl;
    }
    
    if (vec.size() == 1) {
        cur_node->last_modified = MicrosecondsToFileTime(timestamp);
        cur_node->max_last_modified = cur_node->last_modified;
        return;
    }
    cur_node->max_last_modified = max_filetime(cur_node->max_last_modified,
                                               MicrosecondsToFileTime(timestamp));
    
    for (auto child : cur_node->children) {
        if (child->name == vec[1]) {
            std::string next = relative_path.substr(vec[0].size() + 1);
            updateTimestamp(child, next, timestamp);
        }
    }
}

// std::shared_ptr<Node> Node::fromPath(std::wstring abs_path, std::wstring relative_path, bool is_root){
//     std::filesystem::path p(abs_path);
//     if (!std::filesystem::exists(p)) {
//         return nullptr;
//     }
//     auto node = std::make_shared<Node>();
//     node->is_root = is_root;
//     node->abs_path = abs_path;
//     node->last_modified = LONG_LONG_AGO;
//     node->max_last_modified = node->last_modified;
//     node->name = p.filename().wstring();
//     node->relative_path = relative_path + node->name;
//     node->file_type = DIRECTORY;
//     FILETIME tmp_max = node->last_modified;

//     for (const auto& entry : std::filesystem::directory_iterator(abs_path)) {
//         std::wstring tmp_wpath = entry.path().wstring();
//         auto tmp_path = wstr2str(tmp_wpath);
//         if (entry.is_regular_file()) {
//             auto child = std::make_shared<Node>();
//             child->file_type = REGULAR_FILE;
//             child->abs_path = tmp_wpath;
//             child->last_modified = get_last_modified(tmp_wpath);
//             child->max_last_modified = child->last_modified;
//             tmp_max = max_filetime(tmp_max, child->max_last_modified);
//             child->name = entry.path().filename().wstring();
//             child->relative_path = node->relative_path + str2wstr("/") + child->name;
//             node->add_child(child);
//         } else if (entry.is_directory()) {
//             auto toSearch = tmp_wpath;
//             auto child = fromPath(toSearch, node->relative_path + str2wstr("/"), false);
//             tmp_max = max_filetime(tmp_max, child->max_last_modified);
//             node->add_child(child);
//         }
//     }
//     node->max_last_modified = tmp_max;
//     return node;
// }

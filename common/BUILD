cc_library(
    name = "util",
    hdrs = ["util.h"],
    srcs = ["util.cpp"],
    deps = ["@spdlog_repo//:spdlog_lib"],
    visibility = ["//visibility:public"]
)

cc_library(
    name = "tree",
    hdrs = ["tree.h"],
    srcs = ["tree.cpp"],
    deps = ["//protobuf:node_pb", "//common:util"],
    visibility = ["//visibility:public"]
)
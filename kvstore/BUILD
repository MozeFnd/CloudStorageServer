cc_library(
    name = "kvstore",
    srcs = ["kvstore.cpp"],
    hdrs = ["kvstore.h"],
    deps = ["@leveldb_repo//:leveldb", "//common:common"],
    linkopts = ["-Llibs", "-l/home/xun/leveldb/build/libleveldb.a"],
    visibility = ["//visibility:public"]
)
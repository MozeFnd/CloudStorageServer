HEADERS = glob([
    "include/leveldb/*",
])

cc_library(
    name = "leveldb",
    hdrs = HEADERS,
    srcs = ["lib/libleveldb.a"],
    includes = ["include"],
    visibility = ["//visibility:public"]
)
HEADERS = glob([
    "include/leveldb/*",
])

cc_library(
    name = "leveldb",
    hdrs = HEADERS,
    includes = ["include"],
    linkopts = ["-L third_party/level_db/lib"],
    visibility = ["//visibility:public"]
)
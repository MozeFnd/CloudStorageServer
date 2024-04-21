HEADERS = glob([
    "include/leveldb/*",
])

cc_library(
    name = "protobuf",
    hdrs = HEADERS,
    includes = ["src"],
    linkopts = ["-L third_party/protobuf-3.11.2/lib"],
    visibility = ["//visibility:public"]
)
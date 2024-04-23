HEADERS = glob([
    "src/google/protobuf/*",
    "src/google/protobuf/io/*",
    "src/google/protobuf/stubs/*",
])

cc_library(
    name = "protobuf",
    hdrs = HEADERS,
    srcs = ["lib/libprotobuf.a"],
    includes = ["src"],
    #linkopts = ["-L lib"],  # third_party/protobuf-3.11.2/
    visibility = ["//visibility:public"]
)
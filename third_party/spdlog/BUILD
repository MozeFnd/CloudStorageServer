HEADERS = glob([
    "include/spdlog/*",
    "include/spdlog/*/*",
    "include/spdlog/*/*/*",
])

cc_library(
    name = "spdlog_lib",
    hdrs = HEADERS,
    includes = ["include"],
    visibility = ["//visibility:public"]
)
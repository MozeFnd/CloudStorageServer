CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

project(cloud_storage_server)

add_executable(
    server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_executable(
    kvlookup
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kvlookup.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(/home/xun/leveldb/include)
include_directories(/home/xun/spdlog/include)

function(generate_library LIB_NAME)
    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${LIB_NAME}.cpp")
    add_library(${LIB_NAME} STATIC ${SOURCES})
    target_link_libraries(server PUBLIC ${LIB_NAME})
    target_link_libraries(kvlookup PUBLIC ${LIB_NAME})
endfunction()

set(
    libraries
    communicator
    kvstore
    logger
    jsontool
)

foreach(lib ${libraries})
    generate_library(${lib})
endforeach()

target_link_libraries(
    kvstore
    PUBLIC
    /home/xun/leveldb/build/libleveldb.a
)
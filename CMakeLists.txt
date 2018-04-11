cmake_minimum_required(VERSION 3.2)
project(bitcask)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")
find_package(gRPC REQUIRED)


enable_testing()
include_directories(include/bitcask)

add_subdirectory(protobuf)
include_directories(protobuf)
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(test)



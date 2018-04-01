cmake_minimum_required(VERSION 3.2)
project(bitcask)

set(CMAKE_CXX_STANDARD 11)

enable_testing()
include_directories(include/bitcask)
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(test)



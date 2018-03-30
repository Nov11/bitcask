cmake_minimum_required(VERSION 3.9)
project(src/bitcask)

set(CMAKE_CXX_STANDARD 11)

include_directories(include/bitcask)
add_subdirectory(src)
add_subdirectory(include)

add_executable(main main.cpp)
target_link_libraries(main libbitcask)
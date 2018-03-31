add_executable(crc32 crctest.cpp)
target_link_libraries(crc32 libbitcask)

add_executable(pg putandget.cpp)
target_link_libraries(pg libbitcask)
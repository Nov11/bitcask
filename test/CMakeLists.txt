add_executable(crc32 crctest.cpp)
target_link_libraries(crc32 libbitcask)
add_test(crc32test crc32)

add_executable(pg putandget.cpp)
target_link_libraries(pg libbitcask)
add_test(putandgettest pg)


add_executable(hint hintfile.cpp)
target_link_libraries(hint libbitcask)
add_test(hintfiletest hint)


add_executable(roll rolling.cpp)
target_link_libraries(roll libbitcask)
add_test(rollingfiletest roll)

add_executable(gp getandput.cpp)
target_link_libraries(gp libbitcask)
add_test(getnoexistkeytest gp)
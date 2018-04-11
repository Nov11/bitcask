add_executable(crc32 crctest.cpp)
target_link_libraries(crc32 bitcask)
add_test(crc32test crc32)

add_executable(pg putandget.cpp)
target_link_libraries(pg bitcask)
add_test(putandgettest pg)


add_executable(hint hintfile.cpp)
target_link_libraries(hint bitcask)
add_test(hintfiletest hint)

add_executable(gp getandput.cpp)
target_link_libraries(gp bitcask)
add_test(getnoexistkeytest gp)

add_executable(roll rolling.cpp)
target_link_libraries(roll bitcask)
add_test(rollingfiletest roll)


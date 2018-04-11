file(GLOB RPC_SRC "../protobuf/*.cpp")
add_executable(server ${RPC_SRC})
target_link_libraries(server bitcask)
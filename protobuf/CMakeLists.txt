set(protobufconfigdir ${CMAKE_SOURCE_DIR}/protobuf)
add_custom_command(
        OUTPUT ${protobufconfigdir}/BitCask.ph.h ${protobufconfigdir}/BitCask.grpc.ph.h
        COMMAND         cmake -E touch ABCDEFG
        COMMAND         /usr/local/bin/protoc -I ${protobufconfigdir} --cpp_out=${protobufconfigdir} ${protobufconfigdir}/BitCask.proto
        COMMAND         /usr/local/bin/protoc -I ${protobufconfigdir} --grpc_out=${protobufconfigdir} --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin ${protobufconfigdir}/BitCask.proto
        VERBATIM
)

MESSAGE(${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(
        rpcHeader ALL
        DEPENDS ${protobufconfigdir}/BitCask.ph.h ${protobufconfigdir}/BitCask.grpc.ph.h
)

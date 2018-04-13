file(GLOB SRC "*.cpp")
file(GLOB HEADER "${CMAKE_SOURCE_DIR}/include/bitcask/*.h")
set(SOURCE
        ${SRC}
        ${HEADER}
        )

add_library(bitcask ${SOURCE})
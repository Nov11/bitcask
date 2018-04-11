file(GLOB SRC "*.cpp")


set(SOURCE
        ${SRC}
        )

add_library(bitcask ${SOURCE})
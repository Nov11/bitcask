set(SOURCE
        BitCask.cpp
        BitCaskHandle.cpp
        ../include/bitcask/BitCask.h
        ../include/bitcask/BitCaskError.h
        ../include/bitcask/BitCaskHandle.h
        )
add_library(libbitcask ${SOURCE})
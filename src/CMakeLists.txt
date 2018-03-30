set(SOURCE
        BitCask.cpp
        BitCaskHandle.cpp
        CRC32.cpp
        ../include/bitcask/BitCask.h
        ../include/bitcask/BitCaskError.h
        ../include/bitcask/BitCaskHandle.h
        ../include/bitcask/BitCaskLogEntry.h
        ../include/bitcask/BitCaskKeyDir.h
        ../include/bitcask/CRC32.h
        )
add_library(libbitcask ${SOURCE})
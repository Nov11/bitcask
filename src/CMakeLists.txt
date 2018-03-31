set(SOURCE
        BitCask.cpp
        BitCaskHandle.cpp
        BitCaskKeyDir.cpp
        BitCaskLogEntry.cpp
        CRC32.cpp
        ../include/bitcask/BitCask.h
        ../include/bitcask/BitCaskError.h
        ../include/bitcask/BitCaskHandle.h
        ../include/bitcask/BitCaskLogEntry.h
        ../include/bitcask/BitCaskKeyDir.h
        ../include/bitcask/CRC32.h
        ../include/bitcask/Utility.h
        )
add_library(libbitcask ${SOURCE})
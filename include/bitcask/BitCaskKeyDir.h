//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_BITCASKKEYDIR_H
#define SRC_BITCASK_BITCASKKEYDIR_H

#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <BitCaskHandle.h>
#include "BitCaskLogEntry.h"

namespace NS_bitcask {
    template<class Value>
    struct KeyDirEntry {
        std::string file_id;
        size_t value_sz = 0;
        off_t value_pos = 0;
        time_t tstamp = 0;

        template<class Key>
        static KeyDirEntry convert(BitCaskLogEntry<Key, Value> &entry, std::string fileName, off_t value_position);
    };

    template<class Value>
    template<class Key>
    KeyDirEntry<Value> KeyDirEntry<Value>::convert(BitCaskLogEntry<Key, Value> &entry,
                                                   std::string fileName,
                                                   off_t value_position) {
        KeyDirEntry result;
        result.file_id = fileName;
        result.value_sz = entry.value_sz;
        result.tstamp = entry.tstamp;
        result.value_pos = value_position;
        return result;
    }

    template<class Key, class Value>
    class BitCaskKeyDir {
    public:
        std::unordered_map<Key, KeyDirEntry<Value>> hash;

        void rebuild(int openedFD, std::string fileName);
    };

}

#endif //SRC_BITCASK_BITCASKKEYDIR_H

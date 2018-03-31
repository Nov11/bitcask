//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_BITCASKKEYDIR_H
#define SRC_BITCASK_BITCASKKEYDIR_H

#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <BitCaskHandle.h>
#include <map>
#include "BitCaskLogEntry.h"

namespace NS_bitcask {
    struct KeyDirEntry {
        std::string file_id;
        size_t value_sz = 0;
        off_t value_pos = 0;
        time_t tstamp = 0;

        template<class Key, class Value>
        static KeyDirEntry convert(BitCaskLogEntry<Key, Value> &entry) {
            KeyDirEntry result;
            result.file_id = entry.fileName;
            result.value_sz = entry.value_sz;
            result.tstamp = entry.tstamp;
            result.value_pos = entry.value_offset;
            return result;
        }
    };

    template<class Key, class Value>
    class BitCaskKeyDir {
    public:
        std::unordered_map<Key, KeyDirEntry> table;

        void rebuild(int openedFD, std::string fileName) {
            auto entry = BitCaskLogEntry<Key, Value>::readFromFile(openedFD, fileName);
            while (entry != BitCaskLogEntry<Key, Value>::invalid()) {
                table[entry.key] = KeyDirEntry::convert(entry);
                entry = BitCaskLogEntry<Key, Value>::readFromFile(openedFD, fileName);
            }
        }

        void insert(BitCaskLogEntry<Key, Value> &entry) {
            auto ret = KeyDirEntry::convert(entry);
            auto &ref = table[entry.key];
            ref = ret;
        }

        bool get(const Key &key, KeyDirEntry &keyDirEntry) {
            auto iter = table.find(key);
            if (iter != table.end()) {
                keyDirEntry = iter->second;
                return true;
            }
            return false;
        }
    };

}

#endif //SRC_BITCASK_BITCASKKEYDIR_H

//
// Created by c6s on 18-3-31.
//

#include <BitCaskLogEntry.h>
#include <BitCaskKeyDir.h>

namespace NS_bitcask {
    template<class Key, class Value>
    void BitCaskKeyDir<Key, Value>::rebuild(int openedFD, std::string name) {
        off_t off_begin = 0;
        off_t value_position = 0;
        auto entry = BitCaskLogEntry<Key, Value>::readFromFile(openedFD, &off_begin, &value_position);
        while (entry != BitCaskLogEntry<Key, Value>::invalid()) {
            hash[entry.key] = KeyDirEntry<Value>::convert(entry, name, value_position);
        }
    }

    template
    class BitCaskKeyDir<int, std::string>;
}
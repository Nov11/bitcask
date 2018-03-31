//
// Created by c6s on 18-3-31.
//

#include <BitCaskLogEntry.h>
#include <BitCaskKeyDir.h>

namespace NS_bitcask {


//    template<class Key, class Value>
//    void BitCaskKeyDir<Key, Value>::rebuild(int openedFD, std::string name) {
//        auto entry = BitCaskLogEntry<Key, Value>::readFromFile(openedFD, name);
//        while (entry != BitCaskLogEntry<Key, Value>::invalid()) {
//            table[entry.key] = KeyDirEntry::convert(entry);
//        }
//    }
//
//    template<class Key, class Value>
//    void BitCaskKeyDir<Key, Value>::insert(BitCaskLogEntry<Key, Value> &entry) {
//        auto ret = KeyDirEntry::convert(entry);
//        auto &ref = table[entry.key];
//        ref = ret;
//    }
//
//    template<class Key, class Value>
//    bool BitCaskKeyDir<Key, Value>::get(const Key &key, KeyDirEntry &keyDirEntry) {
//        auto iter = table.find(key);
//        if (iter != table.end()) {
//            keyDirEntry = iter->second;
//            return true;
//        }
//        return false;
//    }

//    template
//    class BitCaskKeyDir<int, std::string>;
}
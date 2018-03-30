//
// Created by c6s on 18-3-31.
//

#include <BitCask.h>
#include <BitCaskHandle.h>

namespace NS_bitcask {
template<class Key, class Value>
BitCaskHandle BitCask<Key, Value>::open(const std::string &directoryName, size_t options) {
  return BitCaskHandle();
}
template<class Key, class Value>
BitCaskHandle BitCask<Key, Value>::open(const std::string &directoryName) {
  return BitCaskHandle();
}
template<class Key, class Value>
BitCaskError BitCask<Key, Value>::get(BitCaskHandle handle, const Key &, Value &result) {
  return BitCaskError();
}
template<class Key, class Value>
BitCaskError BitCask<Key, Value>::put(BitCaskHandle handle, const Key &, const Value &) {
  return BitCaskError();
}
template<class Key, class Value>
BitCaskError BitCask<Key, Value>::del(BitCaskHandle handle, const Key &) {
  return BitCaskError();
}
template<class Key, class Value>
BitCaskError BitCask<Key, Value>::merge(const std::string &directoryName) {
  return BitCaskError();
}
template<class Key, class Value>
BitCaskError BitCask<Key, Value>::sync(BitCaskHandle handle) {
  return BitCaskError();
}
template<class Key, class Value>
BitCaskError BitCask<Key, Value>::close(BitCaskHandle handle) {
  return BitCaskError();
}
}


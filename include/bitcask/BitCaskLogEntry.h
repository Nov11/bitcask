//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_BITCASKLOGENTRY_H
#define SRC_BITCASK_BITCASKLOGENTRY_H
#include <cstdint>
#include <ctime>
namespace NS_bitcask {
template<class Key, class Value>
struct BitCaskLogEntry {
  uint32_t crc32;
  time_t tstamp;
  size_t ksz;
  size_t value_sz;
  Key key;
  Value value;
};
}
#endif //SRC_BITCASK_BITCASKLOGENTRY_H

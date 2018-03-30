//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_BITCASKKEYDIR_H
#define SRC_BITCASK_BITCASKKEYDIR_H
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <BitCaskHandle.h>
namespace NS_bitcask {
template<class Value>
struct KeyDirEntry {
  std::string file_id;
  size_t value_sz;
  offset_t value_pos;
  time_t tstamp;
};

template<class Value>
class BitCaskKeyDir {
public:
  std::unordered_map<BitCaskHandle, KeyDirEntry<Value>> hash;
};

}

#endif //SRC_BITCASK_BITCASKKEYDIR_H

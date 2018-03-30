//
// Created by c6s on 18-3-31.
//

#ifndef BITCASK_BITCASK_H
#define BITCASK_BITCASK_H

#include <cstddef>
#include <string>
#include <vector>
#include <BitCaskHandle.h>
#include <BitCaskError.h>
namespace NS_bitcask {
template<class Key, class Value>
class BitCask {
public:
  BitCask() = default;
  BitCask(const BitCask &) = delete;
  BitCask &operator=(const BitCask &) = delete;

  BitCaskHandle open(const std::string &directoryName, size_t options);
  BitCaskHandle open(const std::string &directoryName);
  BitCaskError get(BitCaskHandle handle, const Key &, Value &result);
  BitCaskError put(BitCaskHandle handle, const Key &, const Value &);
  BitCaskError del(BitCaskHandle handle, const Key &);
  BitCaskError list_keys(BitCaskHandle handle, std::vector<int> *result);
//  template<class Func>
//  int fold(BitCaskHandle handle, Func &&func, int acc0);//what is this used for?
  BitCaskError merge(const std::string &directoryName);
  BitCaskError sync(BitCaskHandle handle);
  BitCaskError close(BitCaskHandle handle);
private:
  std::string directory;

};

}

#endif //BITCASK_BITCASK_H

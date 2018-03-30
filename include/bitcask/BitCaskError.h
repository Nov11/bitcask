//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_BITCASKERROR_H
#define SRC_BITCASK_BITCASKERROR_H
#include <string>
namespace NS_bitcask {
struct BitCaskError {
  std::string errorMessage;
  int status;
};
}
#endif //SRC_BITCASK_BITCASKERROR_H

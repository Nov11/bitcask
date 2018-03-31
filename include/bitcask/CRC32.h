//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_CRC32_H
#define SRC_BITCASK_CRC32_H

#include <cstddef>
#include <string>

struct CRC32 {
    static size_t getCRC32(const std::string &s);

    static size_t getCRC32(char *buff, size_t len);
};

#endif //SRC_BITCASK_CRC32_H

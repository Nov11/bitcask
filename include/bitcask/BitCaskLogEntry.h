//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_BITCASKLOGENTRY_H
#define SRC_BITCASK_BITCASKLOGENTRY_H

#include <cstdint>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <string>

namespace NS_bitcask {
    template<class Key, class Value>
    struct BitCaskLogEntry {
        uint32_t crc32 = 0;
        time_t tstamp = 0;
        size_t ksz = 0;
        size_t value_sz = 0;
        Key key;
        Value value;

        off_t value_offset = 0;
        std::string fileName;

        static const BitCaskLogEntry &invalid() {
            static BitCaskLogEntry invalidEntry;
            return invalidEntry;
        }

        static size_t CRC32ANDTIMESTAMPLENGTH;

        static BitCaskLogEntry readFromFile(int fd, const std::string &name);

//        static BitCaskLogEntry readFromFile(int fd) { return readFromFile(fd, nullptr); }

        void writeToFileAndUpdateFields(int fd, const std::string &fileName);

        void writeToFile(int fd);
//  size_t value_offset_in_entry() const {
//    return CRC32ANDTIMESTAMPLENGTH + sizeof(ksz) + sizeof(value_sz) +
//  }

        bool operator==(const BitCaskLogEntry &other) const {
            if (this == &other) {
                return true;
            }
            if (this->crc32 != other.crc32) {
                return false;
            }
            if (this->tstamp != other.tstamp) {
                return false;
            }
            if (this->ksz != other.ksz) {
                return false;
            }
            if (this->value_sz != other.value_sz) {
                return false;
            }
            if (this->key != other.key) {
                return false;
            }
            if (this->value != other.value) {
                return false;
            }
            return true;
        }

        bool operator!=(const BitCaskLogEntry &other) const {
            return !this->operator==(other);
        }
    };

}
#endif //SRC_BITCASK_BITCASKLOGENTRY_H

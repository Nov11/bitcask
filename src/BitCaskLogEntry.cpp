//
// Created by c6s on 18-3-31.
//
#include <unistd.h>
#include <BitCaskLogEntry.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <CRC32.h>

namespace NS_bitcask {

    template<class Key>
    class DESERIAL {
    public:
        static Key deserial(char *buff, size_t);
    };

    template<>
    class DESERIAL<int> {
    public:
        static int deserial(char *buff, size_t len) {
            return *reinterpret_cast<int *>(buff);
        }
    };

    template<>
    class DESERIAL<std::string> {
    public:
        static std::string deserial(char *buff, size_t len) {
            return std::string(buff, len);
        }
    };

    template<class Key>
    class SERIAL {
    public:
        static std::vector<char> serial(const Key &k);
    };

    template<>
    class SERIAL<int> {
    public:
        static std::vector<char> serial(const int &i) {
            std::vector<char> result(sizeof(int));
            *(reinterpret_cast<int *>(&result[0])) = i;
            return result;
        }
    };

    template<>
    class SERIAL<std::string> {
    public:
        static std::vector<char> serial(const std::string &s) {
            return std::vector<char>(s.begin(), s.end());
        }
    };

    template<class Key, class Value>
    BitCaskLogEntry<Key, Value> BitCaskLogEntry<Key, Value>::readFromFile(int fd,
                                                                          off_t *offsetofentry,
                                                                          off_t *offsetOfValue) {
        if (offsetofentry != nullptr) {
            auto curOffset = lseek(fd, 0, SEEK_CUR);
            if (curOffset == -1) {
                perror("get offset of current entry");
                return BitCaskLogEntry::invalid();
            }
        }
        size_t skip = HEADERLENGTH + sizeof(BitCaskLogEntry::ksz)
                      + sizeof(BitCaskLogEntry::value_sz);
        std::vector<char> preamble(skip);
        ssize_t readRet = ::read(fd, preamble.data(), preamble.size());
        if (readRet != preamble.size()) {
            perror("crc & tstamp & ksz & vsz corrupted");
            exit(1);
        }

        size_t ksz = *reinterpret_cast<size_t *>(&preamble[HEADERLENGTH]);
        size_t vsz = *reinterpret_cast<size_t *>(&preamble[HEADERLENGTH + sizeof(BitCaskLogEntry::ksz)]);

        preamble.resize(skip + ksz + vsz);
        readRet = ::read(fd, &preamble[skip], ksz + vsz);
        if (readRet != preamble.size()) {
            perror("read kv content failed");
            exit(1);
        }

        BitCaskLogEntry ret;

        ret.crc32 = *reinterpret_cast<uint32_t *>(&preamble[0]);
        //verify the crc32
        if (CRC32::getCRC32(&preamble[0], preamble.size()) != ret.crc32) {
            perror("invalid crc32 check ignore this record");
            return BitCaskLogEntry::invalid();
        }

        ret.tstamp = *reinterpret_cast<time_t *>(&preamble[sizeof(BitCaskLogEntry::crc32)]);
        ret.ksz = ksz;
        ret.value_sz = vsz;
        ret.key = DESERIAL<Key>::deserial(&preamble[skip], ksz);
        ret.value = DESERIAL<Value>::deserial(&preamble[skip + ksz], vsz);

        if (offsetOfValue != nullptr) {
            *offsetofentry = HEADERLENGTH + sizeof(ret.ksz) + sizeof(ret.value_sz) + ret.ksz;
        }
        return ret;
    }

    template<class Key, class Value>
    void BitCaskLogEntry<Key, Value>::writeToFile(int fd) {
        size_t sz = HEADERLENGTH;
        sz += this->ksz;
        sz += this->value_sz;
        auto kserial = SERIAL<Key>::serial(this->key);
        sz += kserial.size();
        auto vserial = SERIAL<Value>::serial(this->value);
        sz += vserial.size();
        std::vector<char> buff(sz);
        //time
        *(reinterpret_cast<time_t *>(&buff + sizeof(uint32_t))) = time(nullptr);
        *(reinterpret_cast<size_t *>(&buff + HEADERLENGTH)) = this->ksz;
        *(reinterpret_cast<size_t *>(&buff + HEADERLENGTH + sizeof(BitCaskLogEntry::ksz))) = this->value_sz;
        size_t offset = HEADERLENGTH + sizeof(this->ksz) + sizeof(this->value_sz);
        for (size_t i = 0; i < kserial.size(); i++) {
            buff[i + offset] = kserial[i];
        }
        offset += kserial.size();
        for (size_t i = 0; i < vserial.size(); i++) {
            buff[i + offset] = vserial[i];
        }

        ssize_t writeRet = ::write(fd, buff.data(), buff.size());
        if (writeRet != buff.size()) {
            perror("write log entry");
            exit((1));
        }
        return;
    }

    template<class Key, class Value>
    size_t BitCaskLogEntry<Key, Value>::HEADERLENGTH = sizeof(BitCaskLogEntry::crc32) + sizeof(BitCaskLogEntry::tstamp);

    template
    class BitCaskLogEntry<int, std::string>;

}


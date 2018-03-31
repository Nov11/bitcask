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
#include <assert.h>

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
        auto curOffset = lseek(fd, 0, SEEK_CUR);
        if (curOffset == -1) {
            perror("get offset of current entry");
            return BitCaskLogEntry::invalid();
        }

        if (offsetofentry != nullptr) {
            *offsetofentry = curOffset;
        }

        size_t skip = CRC32ANDTIMESTAMPLENGTH + sizeof(BitCaskLogEntry::ksz)
                      + sizeof(BitCaskLogEntry::value_sz);
        std::vector<char> preamble(skip);
        ssize_t readRet = ::read(fd, preamble.data(), preamble.size());
        if (readRet != preamble.size()) {
            perror("crc & tstamp & ksz & vsz corrupted");
            exit(1);
        }

        size_t ksz = *reinterpret_cast<size_t *>(&preamble[CRC32ANDTIMESTAMPLENGTH]);
        size_t vsz = *reinterpret_cast<size_t *>(&preamble[CRC32ANDTIMESTAMPLENGTH + sizeof(BitCaskLogEntry::ksz)]);

        preamble.resize(skip + ksz + vsz);
        readRet = ::read(fd, &preamble[skip], ksz + vsz);
        if (readRet != preamble.size()) {
            perror("read kv content failed");
            exit(1);
        }

        BitCaskLogEntry ret;

        ret.crc32 = *reinterpret_cast<uint32_t *>(&preamble[0]);
        //verify the crc32
        if (CRC32::getCRC32(&preamble[sizeof(BitCaskLogEntry::crc32)],
                            preamble.size() - sizeof(BitCaskLogEntry::crc32)) != ret.crc32) {
            perror("invalid crc32 check ignore this record");
            return BitCaskLogEntry::invalid();
        }

        ret.tstamp = *reinterpret_cast<time_t *>(&preamble[sizeof(BitCaskLogEntry::crc32)]);
        ret.ksz = ksz;
        ret.value_sz = vsz;
        ret.key = DESERIAL<Key>::deserial(&preamble[skip], ksz);
        ret.value = DESERIAL<Value>::deserial(&preamble[skip + ksz], vsz);

        if (offsetOfValue != nullptr) {
            *offsetofentry = curOffset +
                             CRC32ANDTIMESTAMPLENGTH +
                             sizeof(ret.ksz) +
                             sizeof(ret.value_sz) +
                             ret.ksz;
        }
        return ret;
    }

    template<class Key, class Value>
    void BitCaskLogEntry<Key, Value>::writeToFileAndUpdateFields(int fd) {
        size_t sz = CRC32ANDTIMESTAMPLENGTH;
        sz += sizeof(this->ksz);
        sz += sizeof(this->value_sz);
        auto kserial = SERIAL<Key>::serial(this->key);
        sz += kserial.size();
        auto vserial = SERIAL<Value>::serial(this->value);
        sz += vserial.size();
        std::vector<char> buff(sz);


        //get serialize byte array
        //time stamp
        auto tstamp = time(nullptr);
        *(reinterpret_cast<time_t *>(&buff + sizeof(uint32_t))) = tstamp;
        //key size
        *(reinterpret_cast<size_t *>(&buff + CRC32ANDTIMESTAMPLENGTH)) = kserial.size();
        //value size
        *(reinterpret_cast<size_t *>(&buff + CRC32ANDTIMESTAMPLENGTH + sizeof(BitCaskLogEntry::ksz))) = vserial.size();
        size_t offset = CRC32ANDTIMESTAMPLENGTH + sizeof(this->ksz) + sizeof(this->value_sz);
        //key
        for (size_t i = 0; i < kserial.size(); i++) {
            buff[offset++] = kserial[i];
        }
        //value
        for (size_t i = 0; i < vserial.size(); i++) {
            buff[offset++] = vserial[i];
        }
        assert(offset == sz);

        //crc32
        auto crc32 = CRC32::getCRC32(buff.data() + sizeof(BitCaskLogEntry::crc32),
                                     buff.size() - sizeof(BitCaskLogEntry::crc32));


        *(reinterpret_cast<uint32_t *>(buff.data())) = crc32;

        ssize_t writeRet = ::write(fd, buff.data(), buff.size());
        if (writeRet != buff.size()) {
            perror("write log entry");
            exit((1));
        }

        //write fields back to entry object
        this->crc32 = crc32;
        this->tstamp = tstamp;
        this->ksz = kserial.size();
        this->value_sz = vserial.size();

        return;
    }

    template<class Key, class Value>
    size_t BitCaskLogEntry<Key, Value>::CRC32ANDTIMESTAMPLENGTH =
            sizeof(BitCaskLogEntry::crc32) + sizeof(BitCaskLogEntry::tstamp);

    template
    class BitCaskLogEntry<int, std::string>;

}


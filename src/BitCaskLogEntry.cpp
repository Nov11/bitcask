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
#include <Utility.h>

namespace NS_bitcask {

    static off_t currentPosition(int fd) {
        auto curOffset = lseek(fd, 0, SEEK_CUR);
        if (curOffset == -1) {
            perror("get current position in file failed");
            return -1;
        }
        return curOffset;
    }

    static off_t
    get_value_offset(off_t offsetOfEntryHead, size_t crcandtstamp, size_t ks_field, size_t vs_field, size_t ksz) {
        return offsetOfEntryHead +
               crcandtstamp +
               ks_field +
               vs_field +
               ksz;
    }

    template<class Key, class Value>
    BitCaskLogEntry<Key, Value> BitCaskLogEntry<Key, Value>::readFromFile(int fd, const std::string &name) {
        auto curOffset = lseek(fd, 0, SEEK_CUR);
        if (curOffset == -1) {
            perror("get offset of current entry");
            return BitCaskLogEntry::invalid();
        }

        size_t skip = CRC32ANDTIMESTAMPLENGTH + sizeof(BitCaskLogEntry::ksz)
                      + sizeof(BitCaskLogEntry::value_sz);
        std::vector<char> preamble(skip);
        ssize_t readRet = ::read(fd, preamble.data(), preamble.size());
        if (readRet == 0) {
            return BitCaskLogEntry::invalid();
        }
        if (readRet != preamble.size()) {
            perror("crc & tstamp & ksz & vsz corrupted");
            return BitCaskLogEntry::invalid();
        }

        size_t ksz = *reinterpret_cast<size_t *>(&preamble[CRC32ANDTIMESTAMPLENGTH]);
        size_t vsz = *reinterpret_cast<size_t *>(&preamble[CRC32ANDTIMESTAMPLENGTH + sizeof(BitCaskLogEntry::ksz)]);

        preamble.resize(skip + ksz + vsz);
        readRet = ::read(fd, &preamble[skip], ksz + vsz);
        if (readRet != ksz + vsz) {
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

        ret.value_offset = get_value_offset(curOffset, CRC32ANDTIMESTAMPLENGTH, sizeof(ret.ksz), sizeof(ret.value_sz),
                                            ksz);
        ret.fileName = name;
        return ret;
    }


    template<class Key, class Value>
    void BitCaskLogEntry<Key, Value>::writeToFileAndUpdateFields(int fd, const std::string &fileName) {
        auto curPos = currentPosition(fd);
        if (curPos == -1) {
            return;
        }

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
        *(reinterpret_cast<time_t *>(&buff[0] + sizeof(uint32_t))) = tstamp;
        //keyreadBytes size
        *(reinterpret_cast<size_t *>(&buff[0] + CRC32ANDTIMESTAMPLENGTH)) = kserial.size();
        //value size
        *(reinterpret_cast<size_t *>(&buff[0] + CRC32ANDTIMESTAMPLENGTH +
                                     sizeof(BitCaskLogEntry::ksz))) = vserial.size();
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

        this->value_offset = get_value_offset(curPos, CRC32ANDTIMESTAMPLENGTH, sizeof(ksz), sizeof(value_sz), ksz);
        this->fileName = fileName;
        return;
    }

    template<class Key, class Value>
    size_t BitCaskLogEntry<Key, Value>::CRC32ANDTIMESTAMPLENGTH =
            sizeof(BitCaskLogEntry::crc32) + sizeof(BitCaskLogEntry::tstamp);

    template<class Key, class Value>
    void BitCaskLogEntry<Key, Value>::writeToFile(int fd) {
        size_t sz =
                CRC32ANDTIMESTAMPLENGTH + sizeof(BitCaskLogEntry::ksz) + sizeof(BitCaskLogEntry::value_sz) + this->ksz +
                this->value_sz;
        std::vector<char> buff(sz);

        auto kserial = SERIAL<Key>::serial(this->key);
        auto vserial = SERIAL<Value>::serial(this->value);

        //get serialize byte array
        //crc32
        *(reinterpret_cast<uint32_t *>(buff.data())) = this->crc32;
        //time stamp
        *(reinterpret_cast<time_t *>(&buff[0] + sizeof(uint32_t))) = this->tstamp;
        //keyreadBytes size
        *(reinterpret_cast<size_t *>(&buff[0] + CRC32ANDTIMESTAMPLENGTH)) = kserial.size();
        //value size
        *(reinterpret_cast<size_t *>(&buff[0] + CRC32ANDTIMESTAMPLENGTH +
                                     sizeof(BitCaskLogEntry::ksz))) = vserial.size();
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


        ssize_t writeRet = ::write(fd, buff.data(), buff.size());
        if (writeRet != buff.size()) {
            perror("write log entry");
            exit((1));
        }
    }

    template
    class BitCaskLogEntry<int, std::string>;

}


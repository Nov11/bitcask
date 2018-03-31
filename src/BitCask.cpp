//
// Created by c6s on 18-3-31.
//
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

#include <algorithm>
#include <vector>
#include <string>

#include <BitCask.h>
#include <BitCaskHandle.h>
#include <cstring>

namespace NS_bitcask {
    template<class Key, class Value>
    BitCaskHandle BitCask<Key, Value>::open(const std::string &directoryName, size_t options) {
        (void) options;//not sure what tht option should be
        this->directory = directoryName;
//        int fd = ::open(directoryName.data(), O_CREAT | O_RDONLY);
        //create if not exists
        int dir = ::mkdir(directoryName.data(), 0x0777);
        if (dir == -1) {
            //it's alright if there's already one.
            perror("mkdir directory");
        }
        //get the lock to own this directory
        int lock = flock(dir, LOCK_EX | LOCK_NB);
        if (lock == -1) {
            //quit if it cannot get the lock
            perror("flock file");
            return BitCaskHandle::invalid();
        }
        this->dirFd = lock;

        //TODO:read the key directory for this directory and merge it into global tree

        rebuildKeyDirectory();

        //open a file to write kv pairs
        time_t currentTime = time(nullptr);
        BitCaskHandle result(0, std::to_string(currentTime));

        int fileFD = ::open(result.file_name.data(), O_CLOEXEC | O_RDWR, 0x0700);
        if (fileFD != -1) {
            perror("open log file");
        }
        this->fileFd = fileFD;
        return result;
    }

    template<class Key, class Value>
    BitCaskHandle BitCask<Key, Value>::open(const std::string &directoryName) {
        return open(directoryName, 0);
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::get(BitCaskHandle handle, const Key &, Value &result) {
        return BitCaskError::OK();
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::put(BitCaskHandle handle, const Key &, const Value &) {
        BitCaskLogEntry entry()
        return BitCaskError::OK();
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::del(BitCaskHandle handle, const Key &) {
        return BitCaskError::OK();
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::merge(const std::string &directoryName) {
        return BitCaskError::OK();
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::sync(BitCaskHandle handle) {
        return BitCaskError::OK();
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::close(BitCaskHandle handle) {
        return BitCaskError::OK();
    }

    template<class Key, class Value>
    void BitCask<Key, Value>::rebuildKeyDirectory() {
        //iterate all files in this directory
        DIR *dir = fdopendir(dirFd);
        if (dir == NULL) {
            perror("fdopendir");
            exit(1);
        }

        //get a list of all files
        std::vector<std::string> fileNameLists;
        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr) {
            //todo: deal with hint file
            fileNameLists.emplace_back(entry->d_name);
        }
        sort(fileNameLists.begin(), fileNameLists.end());
        //read files from the oldest to newest

        for (auto name : fileNameLists) {
            int fileFd = ::open(name.data(), O_RDONLY);
            if (fileFd == -1) {
                fprintf(stderr, "open file: %s failed : %s\n", name.data(), strerror(errno));
                //should not proceed any more
                //in case something in this file is lost, the later content may be corrupted
                exit(1);
            }
            hash.rebuild(fileFd, name);
        }
    }

    template<class Key, class Value>
    BitCask<Key, Value>::~BitCask() {
        if (this->dirFd != -1) {
            int ret = flock(this->dirFd, LOCK_UN);
            if (ret == -1) {
                perror("unlock directory");
            }
            ::close(this->dirFd);
        }
    }

    template
    class BitCask<int, std::string>;
}


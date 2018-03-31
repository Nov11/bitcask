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
#include <Utility.h>

namespace NS_bitcask {
    struct DIR_WRAPPER {
        DIR *ptr = nullptr;

        DIR_WRAPPER(DIR *ptr) : ptr(ptr) {}

        ~DIR_WRAPPER() {
            if (ptr) {
                closedir(ptr);
            }
        }

        operator DIR *() const {
            return ptr;
        }
    };

    struct FD_WRAPPER {
        int fd = -1;

        FD_WRAPPER(int fd) : fd(fd) {

        }

        ~FD_WRAPPER() {
            if (fd != -1) {
                close(fd);
            }
        }

        operator int() const {
            return fd;
        }
    };

    template<class Key, class Value>
    BitCaskHandle BitCask<Key, Value>::open(const std::string &directoryName, size_t options) {
        (void) options;//not sure what tht option should be
        //create if not exists
        int suc = ::mkdir(directoryName.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (suc != -1) {
            printf("mkdir directory\n");
        }

        //change to that directory
        int chdirRet = chdir(directoryName.data());
        if (chdirRet == -1) {
            perror("change dir");
            return BitCaskHandle::invalid();
        }
        this->directory = directoryName;
        //get the lock to own this directory
        //create a file and lock it
        int lockFile = ::open(lockFileName.data(), O_CREAT, S_IRWXU);
        if (lockFile == -1) {
            perror("create lock file");
            return BitCaskHandle::invalid();
        }

        int flockRet = flock(lockFile, LOCK_EX | LOCK_NB);
        if (flockRet == -1) {
            //quit if it cannot get the lock
            perror("flock file");
            ::close(lockFile);
            return BitCaskHandle::invalid();
        }
        this->lockFileFD = lockFile;

        //TODO:read the key directory for this directory and merge it into global tree

        rebuildKeyDirectory();

        //open a file to write kv pairs
        time_t currentTime = time(nullptr);
        BitCaskHandle result(0, std::to_string(currentTime), this->directory);

        int fileFD = ::open(result.file_name.data(), O_CREAT | O_CLOEXEC | O_RDWR | O_SYNC, S_IRWXU);
        if (fileFD == -1) {
            perror("open log file");
        }
        this->fileFd = fileFD;
        this->currentFileName = result.file_name;
        return result;
    }

    template<class Key, class Value>
    BitCaskHandle BitCask<Key, Value>::open(const std::string &directoryName) {
        return open(directoryName, 0);
    }

    static std::vector<char> readBytes(const std::string &fileName, off_t off, size_t sz) {
        FD_WRAPPER fd = ::open(fileName.data(), O_RDONLY);
        if (fd == -1) {
            perror("read value open file");
            return {};
        }
        off_t ret = lseek(fd, off, SEEK_SET);
        if (ret == -1) {
            perror("read value seek");
            return {};
        }
        std::vector<char> result(sz);
        ssize_t readRet = ::read(fd, result.data(), result.size());
        if (readRet == -1) {
            perror("read value reading ");
            return {};
        }
        if (readRet != result.size()) {
            perror("read value value in file is short than expected");
            return {};
        }
        return result;
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::get(BitCaskHandle handle, const Key &key, Value &result) {
        if (!checkHandle(handle)) {
            return BitCaskError(INVALID_HANDLE, "unmatched handle directory name");
        }
        KeyDirEntry keyDirEntry;
        auto ret = bitCaskKeyDir.get(key, keyDirEntry);
        if (!ret) {
            return BitCaskError(NO_SUCH_ELEMENT_IN_KEY_DIR, "cannot find corresponding key in key directory");
        }

        auto array = readBytes(keyDirEntry.file_id, keyDirEntry.value_pos, keyDirEntry.value_sz);
        result = DESERIAL<Value>::deserial(array.data(), array.size());
        return BitCaskError::OK();
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::put(BitCaskHandle handle, const Key &key, const Value &value) {
        BitCaskLogEntry<Key, Value> entry;
        entry.key = key;
        entry.value = value;
        entry.writeToFileAndUpdateFields(fileFd, handle.file_name);
        bitCaskKeyDir.insert(entry);
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
        DIR_WRAPPER dir = opendir(".");
        if (dir == NULL) {
            perror("fdopendir");
            exit(1);
        }

        //get a list of all files
        std::vector<std::string> fileNameLists;
        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr) {
            //todo: deal with hint file
            if (strcmp(entry->d_name, ".") == 0
                || strcmp(entry->d_name, "..") == 0
                || strcmp(entry->d_name, lockFileName.data()) == 0) {
                continue;
            }
            fileNameLists.emplace_back(entry->d_name);
        }
        sort(fileNameLists.begin(), fileNameLists.end());
        //read files from the oldest to newest

        for (const auto &name : fileNameLists) {
            FD_WRAPPER fileFd = ::open(name.data(), O_RDONLY);
            if (fileFd == -1) {
                fprintf(stderr, "open file: %s failed : %s\n", name.data(), strerror(errno));
                //should not proceed any more
                //in case something in this file is lost, the later content may be corrupted
                exit(1);
            }
            bitCaskKeyDir.rebuild(fileFd, name);
        }
    }

    template<class Key, class Value>
    BitCask<Key, Value>::~BitCask() {
        if (this->lockFileFD != -1) {
            int ret = flock(this->lockFileFD, LOCK_UN);
            if (ret == -1) {
                perror("unlock directory");
            }
            ::close(this->lockFileFD);
        }
        if (this->fileFd != -1) {
            ::close(this->fileFd);
        }
        chdir("..");
    }

    template
    class BitCask<int, std::string>;
}


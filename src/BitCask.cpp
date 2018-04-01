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
#include <assert.h>
#include <set>

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
        auto outputFileName = newFileName();
        int fileFD = ::open(outputFileName.data(), O_CREAT | O_CLOEXEC | O_RDWR | O_SYNC, S_IRWXU);
        if (fileFD == -1) {
            perror("open log file");
            return BitCaskHandle::invalid();
        }
        this->fileFd = fileFD;
        this->currentFileName = outputFileName;
        this->opened = true;

        BitCaskHandle result(0
//                , outputFileName
                , this->directory);
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
        (void) handle;
        BitCaskLogEntry<Key, Value> entry;
        entry.key = key;
        entry.value = value;
        rollingFileIfNeeded();
        entry.writeToFileAndUpdateFields(fileFd, currentFileName);
        bitCaskKeyDir.insert(entry);
        return BitCaskError::OK();
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::del(BitCaskHandle handle, const Key &key) {
        (void) handle;
        //remove key from key entry directory and mark that in file
        //if a key is removed, its value sz will be empty
        //first mark it in file
        BitCaskLogEntry<Key, Value> entry;
        entry.key = key;
        entry.value_sz = 0;
        rollingFileIfNeeded();
        entry.writeToFileAndUpdateFields(fileFd, currentFileName);
        //second remove it from directory
        bitCaskKeyDir.del(key);
        return BitCaskError::OK();
    }

    /**
     * Merge several data files within a Bitcask datastore into a more
     * compact form. Also, produce hintfiles for faster startup.
     * @tparam Key
     * @tparam Value
     * @param directoryName
     * @return
     */
    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::merge(const std::string &directoryName) {
        assert(directoryName == this->directory);
        //transverse files in this folder
        DIR_WRAPPER dir = opendir(".");
        if (dir == nullptr) {
            perror("open dir for merge");
            return BitCaskError(CANNOT_OPEN_DIR, "cannot open current dir");
        }

        dirent *dirEntry = readdir(dir);
        while (dirEntry) {
            if (strcmp(".", dirEntry->d_name) == 0
                || strcmp("..", dirEntry->d_name) == 0
                //                || strcmp(currentFileName.data(), dirEntry->d_name) == 0
                || strcmp(lockFileName.data(), dirEntry->d_name) == 0) {
                //nothing
            } else {
                doMerge(dirEntry->d_name);
            }


            dirEntry = readdir(dir);
        }

        return BitCaskError::OK();
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::sync(BitCaskHandle handle) {
        //as using O_SYNC flag on opened files, this is not needed
        return BitCaskError::OK();
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::close(BitCaskHandle handle) {
        this->opened = false;
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
        std::set<std::string> fileNameLists;
        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0
                || strcmp(entry->d_name, "..") == 0
                || strcmp(entry->d_name, lockFileName.data()) == 0) {
                continue;
            }

            std::string n(entry->d_name);
            if (n.size() > 4) {
                std::string postfix = n.substr(n.size() - 4);
                if (postfix == "hint") {
                    fileNameLists.insert(entry->d_name);
                    std::string prefix = n.substr(0, n.size() - 4);
                    fileNameLists.erase(prefix);
                } else {
                    std::string hintFile = n + "hint";
                    if (fileNameLists.find(hintFile) == fileNameLists.end()) {
                        fileNameLists.insert(entry->d_name);
                    }
                }
            } else {
                assert(false);
            }
        }
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
        chdir(originalDirectory.data());
    }

    template<class Key, class Value>
    BitCask<Key, Value>::BitCask() {
        std::vector<char> buff(100);
        getcwd(&buff[0], buff.size());
        for (int i = 0; i < buff.size() && buff[i] != '\0'; i++) {
            originalDirectory.push_back(buff[i]);
        }
    }

    template<class Key, class Value>
    BitCaskError BitCask<Key, Value>::list_keys(BitCaskHandle handle, std::vector<Key> *result) {
        assert(this->opened == true);
        auto ret = bitCaskKeyDir.keyList();
        std::copy(ret.begin(), ret.end(), back_inserter(*result));
        return BitCaskError::OK();
    }

    void appendToFile(int fd, const char *buff, size_t sz) {
        ssize_t retWrite = ::write(fd, buff, sz);
        if (retWrite != sz) {
            perror("write bytes less than expected");
        }
    }

    /**
     * look through every log entry
     * if the entry is still in keyDirectory, keep the file and write it to a hint file, create a new one if there's none
     * else ignore it
     *
     * if the whole file is read and no hint file is generated, remove the file.
     * if there is already a hint file, remove it as well.
     *
     *
     *
     * @tparam Key
     * @tparam Value
     * @param name
     */
    template<class Key, class Value>
    void BitCask<Key, Value>::doMerge(const std::string &name) {
        FD_WRAPPER hintFD = ::open((name + "hint.tmp").data(), O_WRONLY | O_CREAT, S_IRWXU);
        if (hintFD == -1) {
            perror("open hint tmp file failed");
            return;
        }

        FD_WRAPPER fileFD = ::open(name.data(), O_RDONLY);
        if (fileFD == -1) {
            perror("open original file failed");
        }

        bool notEmpty = false;
        auto logEntry = BitCaskLogEntry<Key, Value>::readFromFile(fileFD, name);
        while (logEntry != BitCaskLogEntry<Key, Value>::invalid()) {
            //is this log in key directory?
            KeyDirEntry keyDirEntry;
            auto ret = bitCaskKeyDir.get(logEntry.key, keyDirEntry);
            if (ret) {
                if (keyDirEntry.value_pos == logEntry.value_offset) {
                    //log this log entry to hint file
                    rollingFileIfNeeded();
                    logEntry.writeToFile(hintFD);
                    notEmpty = true;
                }
            }
            logEntry = BitCaskLogEntry<Key, Value>::readFromFile(fileFD, name);
        }

        if (!notEmpty) {
            //remove this & its possible hint file & tmp hint file
            unlink(name.data());
            unlink((name + "hint").data());
            unlink((name + "hint.tmp").data());
        } else {
            unlink((name + "hint").data());
            int ret = rename((name + "hint.tmp").data(), (name + "hint").data());
            if (ret == -1) {
                perror("rename hint tmp file to hint file failed");
            }
        }
    }

    template<class Key, class Value>
    void BitCask<Key, Value>::rollingFileIfNeeded() {
        assert(opened == true);
        //get the file length of fileFd
        struct stat stat1;
        if (::stat(currentFileName.data(), &stat1) != 0) {
            perror("get length of current writing file");
            return;
        }

        if (stat1.st_size > logFileLengthLimit) {
            //close the current file
            ::close(fileFd);
            this->currentFileName = "";
            //open a new one
            auto name = newFileName();
            int ret = ::open(name.data(), O_CREAT | O_CLOEXEC | O_RDWR | O_SYNC, S_IRWXU);
            if (ret == -1) {
                perror("create rolling log file");
                return;
            }
            this->currentFileName = name;
            this->fileFd = ret;
        }
    }

    template
    class BitCask<int, std::string>;
}


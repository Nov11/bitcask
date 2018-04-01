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
#include <BitCaskKeyDir.h>

namespace NS_bitcask {

/**
 * one instance for one directory
 * manage writing/reading files and a key directory
 *
 * should use multiple instance if provide service via many folders
 *
 * @tparam Key key type i.e. integer value
 * @tparam Value  value type i.e. serialized string value
 */
    template<class Key, class Value>
    class BitCask {
    public:
        BitCask();

        ~BitCask();

        BitCask(const BitCask &) = delete;

        BitCask &operator=(const BitCask &) = delete;

        BitCaskHandle open(const std::string &directoryName, size_t options);

        BitCaskHandle open(const std::string &directoryName);

        BitCaskError get(BitCaskHandle handle, const Key &, Value &result);

        BitCaskError put(BitCaskHandle handle, const Key &, const Value &);

        BitCaskError del(BitCaskHandle handle, const Key &);

        BitCaskError list_keys(BitCaskHandle handle, std::vector<Key> *result);

//  template<class Func>
//  int fold(BitCaskHandle handle, Func &&func, int acc0);//what is this used for?
        BitCaskError merge(const std::string &directoryName);

        BitCaskError sync(BitCaskHandle handle);

        BitCaskError close(BitCaskHandle handle);

    private:
        //fired up and ready for service
        bool opened = false;
        //working directory' parent directory. recovery when bitcask destructed
        std::string originalDirectory;
        //working directory
        std::string directory;
        //use this lock file to make sure that there's only one thread take charge of this directory
        std::string lockFileName = "curLockFile";
        //lock file's file descriptor
        int lockFileFD = -1;
        //the file that the current writing goes to
        int fileFd = -1;
        //the file name for fileFd
        std::string currentFileName;
        //in memory index from a key to its newest copy's location
        BitCaskKeyDir<Key, Value> bitCaskKeyDir;
        //if file length exceeded, create a new file. in bytes.
        const size_t logFileLengthLimit = 512;

        void rebuildKeyDirectory();

        void doMerge(const std::string &name);

        bool checkHandle(const BitCaskHandle &handle) { return handle.directory == directory; }

        void rollingFileIfNeeded();

        static std::string newFileName() {
            time_t currentTime = time(nullptr);
            return std::to_string(currentTime);
        }
    };

}

#endif //BITCASK_BITCASK_H

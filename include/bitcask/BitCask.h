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
        bool opened = false;
        std::string originalDirectory;
        std::string directory;
        std::string lockFileName = "curLockFile";
        int lockFileFD = -1;
        int fileFd = -1;
        std::string currentFileName;
        BitCaskKeyDir<Key, Value> bitCaskKeyDir;

        void rebuildKeyDirectory();

        void doMerge(const std::string& name);
        bool checkHandle(const BitCaskHandle &handle) { return handle.directory == directory; }
    };

}

#endif //BITCASK_BITCASK_H

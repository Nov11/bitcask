//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_BITCASKHANDLE_H
#define SRC_BITCASK_BITCASKHANDLE_H

#include <string>
#include <utility>

namespace NS_bitcask {
    struct BitCaskHandle {
        int number = -1;
        std::string file_name;

        static const BitCaskHandle &invalid() {
            static BitCaskHandle invalid;
            return invalid;
        }

        BitCaskHandle() = default;

        BitCaskHandle(int n, std::string fileName) : number(n), file_name(std::move(fileName)) {}
    };

}

#endif //SRC_BITCASK_BITCASKHANDLE_H

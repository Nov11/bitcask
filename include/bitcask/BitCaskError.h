//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_BITCASKERROR_H
#define SRC_BITCASK_BITCASKERROR_H

#include <string>

namespace NS_bitcask {
    struct BitCaskError {
        int errorCode = -1;
        std::string errorMessage;

        static BitCaskError &OK() {
            static BitCaskError OK(-1, "");
            return OK;
        }

        BitCaskError(int errorCode, const std::string &msg) : errorCode(errorCode), errorMessage(msg) {}
    };

}
#endif //SRC_BITCASK_BITCASKERROR_H

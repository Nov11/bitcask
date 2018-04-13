//
// Created by c6s on 18-3-31.
//

#ifndef SRC_BITCASK_BITCASKERROR_H
#define SRC_BITCASK_BITCASKERROR_H

#include <string>
#include <utility>

namespace NS_bitcask {
    enum {
        SUCCESS = 0,
        INVALID_HANDLE,
        NO_SUCH_ELEMENT_IN_KEY_DIR,
        CANNOT_OPEN_DIR,
        ERROR_FLUSH_BUFFER
    };

    struct BitCaskError {
        int errorCode = -1;
        std::string errorMessage;

        static BitCaskError &OK() {
            static BitCaskError OK(SUCCESS, "OK");
            return OK;
        }

        BitCaskError(int errorCode, std::string msg) : errorCode(errorCode), errorMessage(std::move(msg)) {}

        bool operator==(const BitCaskError &other) const {
            if (this == &other) {
                return true;
            }
            return errorCode == other.errorCode && errorMessage == other.errorMessage;
        }
    };

}
#endif //SRC_BITCASK_BITCASKERROR_H

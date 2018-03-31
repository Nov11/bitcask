//
// Created by c6s on 18-4-1.
//

#ifndef SRC_BITCASK_UTILITY_H
#define SRC_BITCASK_UTILITY_H
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

}
#endif //SRC_BITCASK_UTILITY_H

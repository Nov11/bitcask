#include <iostream>
#include <string>
#include <BitCask.h>
#include <BitCaskHandle.h>

using namespace std;
using namespace NS_bitcask;
NS_bitcask::BitCask<int, std::string> bitCask;

std::string checkValue(BitCaskHandle &handle, int key) {
    std::string value;
    auto ret = bitCask.get(handle, key, value);
    std::cout << "=============" << std::endl;

    std::cout << "error code: " << ret.errorCode << " reason: " << ret.errorMessage << std::endl;
    std::cout << "value : " << value << std::endl;

    std::cout << "=============" << std::endl;
    return value;
}

int main() {
    auto handle = bitCask.open("bctest");
    if (!handle.OK()) {
        return 1;
    }
    auto v = checkValue(handle, 1);
    if (v.empty()) {
        v.push_back('A');
    } else {
        v.push_back(v.back() + 1);
    }
    bitCask.put(handle, 1, v);
    checkValue(handle, 1);

    return 0;
}
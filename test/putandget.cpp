//
// Created by c6s on 18-4-1.
//


#include <BitCask.h>
#include <iostream>
#include <assert.h>
#include <dirent.h>

using namespace std;
using namespace NS_bitcask;

int main() {
    vector<char> buff(100);
    getcwd(&buff[0], 100);
    DIR *dir = opendir("bctest");
    if (dir) {
        chdir("bctest");
        dirent *entry = readdir(dir);
        while (entry) {
            cout << entry->d_name << endl;
            if (unlink(entry->d_name)) {
                perror("unlink");
            }
            entry = readdir(dir);
        }
        chdir("..");
    }
    closedir(dir);

    map<int, string> values;
    char c = 'A';
    for (int i = 0; i < 10; i++) {
        values[i] = to_string(c);
        c++;
    }

    {
        BitCask<int, string> bitCask;
        auto handle = bitCask.open("bctest");
        if (!handle.OK()) {
            return -1;
        }


        for (auto item : values) {
            assert(bitCask.put(handle, item.first, item.second) == BitCaskError::OK());
        }

        for (auto item : values) {
            string tmp;
            assert(bitCask.get(handle, item.first, tmp) == BitCaskError::OK());
            assert(item.second == tmp);
        }

        //this is for delete
        bitCask.del(handle, 5);
        values.erase(5);
        //

        bitCask.close(handle);
    }
    {
        BitCask<int, string> bitCask;
        auto handle = bitCask.open("bctest");
        if (!handle.OK()) {
            return -1;
        }

        for (auto item : values) {
            string tmp;
            assert(bitCask.get(handle, item.first, tmp) == BitCaskError::OK());
            assert(item.second == tmp);
        }

    }

    return 0;
}
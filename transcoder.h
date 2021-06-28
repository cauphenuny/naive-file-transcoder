/*
 * file:   transcode.h
 * author: starseekist (https://github.com/starseekist)
 *
 * last modified: Wed June 16 2021 0:06 GMT+0800 (CST)
 */

#pragma once
#include <string>
#include <fstream>

using std::string;

typedef unsigned long long u64_t;
typedef   signed long long s64_t;

u64_t file_getsize(const string& file_name) {
    std::ifstream in(file_name.c_str());
    in.seekg(0, std::ios::end);
    u64_t size = in.tellg();
    in.close();
    return size;
}

bool file_exist(const string& file_name) {
    FILE *p = fopen(file_name.c_str(), "r");
    return p != NULL;
}

void file_clear(const string& file_name) {
    if (file_exist(file_name)) {
        std::ofstream out(file_name);
        out.clear();
    }
}

string suffix(const string& str, const char& ch = '.') {
    int pos = -1;
    for (int i = 0, l = str.length(); i < l; i++) if (str[i] == ch) pos = i;
    return str.substr(pos + 1);
}

string prefix(const string& str, const char& ch = '.') {
    string res;
    for (int i = 0, l = str.length(); i < l; i++) {
        if (str[i] == ch) res = str.substr(0, i);
    }
    return res;
}


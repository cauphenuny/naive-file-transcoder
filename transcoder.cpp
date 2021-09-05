/*
 * transcode.cpp
 * Copyright (c) hydropek 2021. All rights reserved.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <cmath>
#include <cstdio>
#include <cassert>

#include "libini.h"
#include "libcrc32.h"

#if defined(_WIN32) || defined(_WIN64)
    #define PATH_CHAR '\\'
#else
    #define PATH_CHAR '/'
#endif

using namespace std;

constexpr char version[] = "v1.1.0";

typedef unsigned long long u64_t;
typedef   signed long long s64_t;

void print_info();
u64_t file_getsize(const string&);
bool file_exist(const string&);
void file_clear(const string&);
string suffix(const string&, const char& = '.');
string prefix(const string&, const char& = '.');

constexpr u64_t split_limit = 2048;
constexpr u64_t buffer_size = 1048576;
constexpr int key[] = {114, 5, 14, 19, 26, 0, 81, 7},
              key_size = sizeof(key) / sizeof(decltype(key[0]));
int key_pos;
fstream infile, outfile;
CRC32 crc32_origin, crc32_encrypt;

void transcode(const string& input, const string& output, int limit = -1) { // main process
    cout << "[" << suffix(input, PATH_CHAR) << "] ==> [" << suffix(output, PATH_CHAR) << "] ...";
    cout.flush();
    char buffer[buffer_size + 10];
    for (int i = 0; limit < 0 || i < limit; i++) {
        infile.read((char*)&buffer, buffer_size);
        int readed_bytes = infile.gcount();
        if (readed_bytes == 0) break;
        crc32_origin.add(buffer, readed_bytes);
        for (int i = 0; i < readed_bytes; i++) {
            buffer[i] ^= (char)key[key_pos];
            key_pos = (key_pos + 1) % key_size;
        }
        crc32_encrypt.add(buffer, readed_bytes);
        outfile.write(buffer, readed_bytes);
    }
    cout << " ok" << endl;
}

int decode(string output) {
    if (regex_match(suffix(output), regex("p[0-9]+"))) { // match *.p*.trs 
        output = prefix(output);
    }
    INI metadata(output + ".meta");
    
    {
        cout << "verifing key ... ";
        if (!metadata.is_open()) {
            cout << "failed: no metadata" << endl << endl;
        } else {
            cout << "ok" << endl;
            string tmp;
            if ((tmp = metadata.get<string>("metadata", "version")) != (string)version) {
                printf("Warning: version not matched: %s != %s\n", tmp.c_str(), version);
            }
            CRC32 crc32;
            crc32.add(key, sizeof(key));
            if ((tmp = metadata.get<string>("verification", "key")) != crc32.getHash()) {
                printf("\nFATAL: key not matched: expected %s, but find %s.\n", crc32.getHash().c_str(), tmp.c_str());
                return -1;
            }
        }
    }
    
    {
        vector<string> input_list; // candidate input list
        if (file_exist(output + ".p0.trs")) {
            for (int i = 0; ; i++) {
                string input = output + ".p" + to_string(i) + ".trs";
                if (!file_exist(input)) break;
                input_list.push_back(input);
            }
        } else {
            input_list.push_back(output + ".trs");
        }
        file_clear(output);
        outfile.open(output, ios::out | ios::binary | ios::app);
        if (input_list.size() > 1) {
            cout << endl << "candidate input list:" << endl;
            for (size_t i = 0; i < input_list.size(); i++) {
                cout << "#" << i + 1 << " " << suffix(input_list[i], PATH_CHAR) << endl;
            }
            cout << endl << endl;
        }
        for (auto input : input_list) {
            infile.open(input, ios::in | ios::binary);
            crc32_origin.reset();
            transcode(input, output);
            infile.close();
        }
        outfile.close();
    }
    
    {
        cout << "verifying crc-32 ...";
        string cur = crc32_encrypt.getHash(), pre = "";
        if (metadata.is_open()) {
            pre = metadata.get<string>("verification", "crc32");
        }
        if (pre == "" || cur == pre) {
            cout << " ok, " + cur << endl;
        } else {
            cout << " failed!" << endl << endl;
            cout << "FATAL: crc-32 verification failed on [" + suffix(output, PATH_CHAR) + "]" << endl;
            cout << "   ==> expected " + cur + ", but find " + pre << "." << endl;
            return -2;
        }
    }
    return 0;
}

int encode(const string& input) {
    INI metadata(input + ".meta");
    
    { // verification
        cout << ("creating metadata [" + suffix(input, PATH_CHAR) + ".meta" + "] ...");
        file_clear(input + ".meta");
        metadata.put("metadata", "version", version);
        CRC32 crc32;
        crc32.add(key, sizeof(key));
        metadata.put("verification", "key", crc32.getHash());
        cout << " ok" << endl << endl;
    }
    
    
    { // output
        vector<string> output_list; // candidate output list
        if (file_getsize(input) > split_limit * buffer_size) {
            int split_count = std::ceil(1.0 * file_getsize(input) / (split_limit * buffer_size));
            for (int i = 0; i < split_count; i++) {
                output_list.push_back(input + ".p" + to_string(i) + ".trs");
            }
        } else {
            output_list.push_back(input + ".trs");
        }
        infile.open(input, ios::in | ios::binary);
        for (auto output : output_list) {
            outfile.open(output, ios::out | ios::binary);
            crc32_encrypt.reset();
            transcode(input, output, split_limit);
            outfile.close();
        }
        infile.close();
    }
    
    { // calc crc32
        cout << "calculating crc32 ...";
        string res = crc32_origin.getHash();
        cout << " ok, " + res << endl;
        metadata.put("verification", "crc32", res);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    string file_name;
    if (argc != 2) 
        cout << "input filename: ", getline(cin, file_name), cout << endl;
    else 
        file_name = argv[1];
    if (!file_exist(file_name)) {
        cout << "no such file: " << file_name << endl;
        while (1);
    }
    int ret = 0;
    if (suffix(file_name) == "trs" || suffix(file_name) == "meta") {
        ret = decode(prefix(file_name));
    } else {
        ret = encode(file_name);
    }
    if (!ret) cout << endl << "completed!";
    else cout << endl << "terminated with return status " << ret << ".";
    getchar();
    return 0;
}

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
    std::ofstream out(file_name);
    out.clear();
}

string suffix(const string& str, const char& ch) {
    int pos = -1;
    for (int i = 0, l = str.length(); i < l; i++) if (str[i] == ch) pos = i;
    return str.substr(pos + 1);
}

string prefix(const string& str, const char& ch) {
    string res;
    for (int i = 0, l = str.length(); i < l; i++) {
        if (str[i] == ch) res = str.substr(0, i);
    }
    return res;
}

void print_info() {
    cout << "transcoder " << version << endl;
    cout << "repo: https://github.com/hydropek/naive-file-transcoder" << endl;
    cout << "-----------------------------" << endl << endl;
}


/*
 * file:   transcode.cpp
 * author: starseekist (https://github.com/starseekist)
 *
 * last modified: Mon June 28 2021 23:14 GMT+0800 (CST)
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <cmath>
#include <cstdio>
#include <cassert>

#include "transcoder.h"

using namespace std;

constexpr char version[] = "v1.0";

void print_info() {
    cout << "transcoder " << version << endl;
	cout << "repo: https://github.com/starseekist/naive-file-transcoder" << endl;
    cout << "-----------------------------" << endl << endl;
}

constexpr u64_t split_limit = 2048;
constexpr u64_t buffer_size = 1048576; // don't change it
constexpr int key[] = {114, 5, 14, 19, 26, 0, 81, 7}, // don't change it
			  key_size = sizeof(key) / sizeof(int);
int key_pos;
fstream infile, outfile;

void transcode(const string& input, const string& output, int limit = -1) { // main process
    cout << "[" << suffix(input, '\\') << "] ==> [" << suffix(output, '\\') << "] ...";
    char buffer[buffer_size + 10];
    for (int i = 0; limit < 0 || i < limit; i++) {
		infile.read((char*)&buffer, buffer_size);
	    int readed_bytes = infile.gcount();
		if (readed_bytes == 0) break;
	    for (int i = 0; i < readed_bytes; i++) {
	    	buffer[i] ^= (char)key[key_pos];
	    	key_pos = (key_pos + 1) % key_size;
		}
		outfile.write(buffer, readed_bytes);
	}
	cout << " ok" << endl;
}

void decode(string output) {
	vector<string> cand_input; // candidate input list
	if (regex_match(suffix(output), regex("p[0-9]+"))) { // match *.p*.trs 
		output = prefix(output);
		for (int i = 0; ; i++) {
			string input = output + ".p" + to_string(i) + ".trs";
			if (!file_exist(input)) break;
			cand_input.push_back(input);
		}
	} else {
		cand_input.push_back(output + ".trs");
	}
	file_clear(output);
	outfile.open(output, ios::out | ios::binary | ios::app);
	if (cand_input.size() > 1) {
		cout << "candidate input list:" << endl;
		for (size_t i = 0; i < cand_input.size(); i++) {
			cout << "#" << i + 1 << " " << suffix(cand_input[i], '\\') << endl;
		}
		cout << endl << endl;
	}
	for (auto input : cand_input) {
		infile.open(input, ios::in | ios::binary);
		transcode(input, output);
		infile.close();
	}
	outfile.close();
}

void encode(const string& input) {
	vector<string> cand_output; // candidate output list
	if (file_getsize(input) > split_limit * buffer_size) {
		int split_count = std::ceil(1.0 * file_getsize(input) / (split_limit * buffer_size));
		for (int i = 0; i < split_count; i++) {
			cand_output.push_back(input + ".p" + to_string(i) + ".trs");
		}
	} else {
		cand_output.push_back(input + ".trs");
	}
	infile.open(input, ios::in | ios::binary);
	for (auto output : cand_output) {
		outfile.open(output, ios::out | ios::binary);
		transcode(input, output, split_limit);
		outfile.close();
	}
	infile.close();
}

int main(int argc, char* argv[]) {
	print_info();
    string file_name;
    if (argc != 2) 
        cout << "input filename: ", getline(cin, file_name);
    else 
        file_name = argv[1];
    if (!file_exist(file_name)) {
        cout << "no such file: " << file_name << endl;
        while (1);
    }
    if (suffix(file_name) == "trs") {
    	decode(prefix(file_name));
    } else {
    	encode(file_name);
    }
    cout << "\ncompleted!";
    getchar();
    return 0;
}


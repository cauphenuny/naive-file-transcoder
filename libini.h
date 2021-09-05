#pragma once

#include <string>
#include <cstdio>
using std::string;
using std::stringstream;

#define BUFFER_SIZE 1024

int getinikeystring(const char *title, const char *key, const char *filename, char *buf);
int putinikeystring(const char *title, const char *key, const char *val, const char *filename);
int getinikeyint(const char *title, const char *key, const char *filename);
long long getinikeylong(const char *title, const char *key, const char *filename);
double getinikeyfloat(const char *title, const char *key, const char *filename);
int libini_memory(char **pp);
void libini_free(char *p);

class INI {
private:
    char *tmp;
    string filename;
    bool _is_open = 0;
    void check_open() {
        FILE *fp;
        _is_open = ((fp = fopen(filename.c_str(), "r")) != NULL);
    }
public:
    void open(const string &str) {
        libini_memory(&tmp);
        filename = str;
        check_open();
    }

    INI(const char* str) { open(str); }
    INI(const string &str) { open(str); }
    INI() { _is_open = 0; }
    ~INI() { libini_free(tmp); }
    bool is_open() { return _is_open; }

    template<typename T>
    T get(const string &title, const string &key) {
        char buf[BUFFER_SIZE];
        getinikeystring(title.c_str(), key.c_str(), filename.c_str(), buf);
        string value = buf;
        T result;
        stringstream sstr(value);
        sstr >> result;
        return result;
    }
    int put(const string &title, const string &key, const string &val) {
        return putinikeystring(title.c_str(), key.c_str(), val.c_str(), filename.c_str());
    }
};


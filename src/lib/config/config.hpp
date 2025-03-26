#pragma once
#include <string>
#include <map>
using namespace std;
#define CONFIG_FILE "data/config.conf" // relative to home path
#include <vector>


class Config {
    map<string, string> kv;
public:
    Config();
    std::string home_path;
    bool exist(const string& key);
    string get(const string& key);
    long long get_int(const string& key);
    double get_double(const string& key);
    long long get_timestamp(const string& key);
    std::string get_path(const string& key);
    std::vector<std::string> get_csv_strings(const string& key);
    bool get_bool(const string& key);
    void load();

};

extern Config config;

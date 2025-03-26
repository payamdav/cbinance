#include "config.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <time.h>
#include "../utils/file_utils.hpp"
#include "../utils/string_utils.hpp"
#include "../utils/datetime_utils.hpp"
#include <cstdlib>
#include <string>


using namespace std;


Config::Config() {
    home_path = getenv("HOME");
    if (home_path.empty()) {
        cout << "HOME environment variable not set" << endl;
        exit(1);
    }
    load();
}

void Config::load() {
    kv.clear();
    if (!utils::is_file_exists(home_path + "/" + CONFIG_FILE)) {
        cout << "Config File not exists: " << home_path + "/" + CONFIG_FILE << endl;
    } else {
        // cout << "Config File exists: " << home_path + "/" + CONFIG_FILE << endl;
        ifstream file(home_path + "/" + CONFIG_FILE, ios::in);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                if (line[0] == '#') {
                    continue;
                }
                size_t pos = line.find("=");
                if (pos == string::npos) {
                    continue;
                }
                string key, value;
                tie(key, value) = utils::split(line, "=");
                kv[utils::trim(key)] = utils::trim(value);
            }
            file.close();
        }
    }
}


string Config::get(const string& key) {
    if (kv.find(key) == kv.end()) {
        return "";
    }
    return kv[key];
}

bool Config::exist(const string& key) {
    return kv.find(key) != kv.end();
}

long long Config::get_int(const string& key) {
    if (kv.find(key) == kv.end()) {
        return 0;
    }
    return stoll(kv[key]);
}

double Config::get_double(const string& key) {
    if (kv.find(key) == kv.end()) {
        return 0;
    }
    return stod(kv[key]);
}

long long Config::get_timestamp(const string& key) {
    if (kv.find(key) == kv.end()) {
        return 0;
    }
    string value = kv[key];
    return utils::get_timestamp(value);
}

string Config::get_path(const string& key) {
    if (kv.find(key) == kv.end()) {
        return "";
    }
    // if path does not start with /, add home path
    if (kv[key][0] != '/') {
        return home_path + "/" + kv[key];
    }
    return kv[key];
}

bool Config::get_bool(const string& key) {
    if (kv.find(key) == kv.end()) {
        return false;
    }
    return kv[key] == "true";
}

vector<string> Config::get_csv_strings(const string& key) {
    if (kv.find(key) == kv.end()) {
        return {};
    }
    return utils::split_to_vector(kv[key], ",");
}

Config config;

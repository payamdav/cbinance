#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "../../config/config.hpp"
#include <unordered_map>


using namespace std;

namespace ob {

const size_t SNAPSHOT_IDX_BYTES = 48;
const size_t UPDATE_IDX_BYTES = 64;


class Symbol {
    public:
        Symbol(string symbol);
        ~Symbol();
        void get_last_update_id();

        string symbol;
        string base_path;

        ofstream snapshot_data;
        ofstream snapshot_idx;
        ofstream update_data;
        ofstream update_idx;

        size_t snapshot_last_u_id;
        size_t update_last_u_id;


};

class SnapshotIdx {
    public:
        size_t t;
        size_t u_id;
        size_t offset;
        size_t size;
        size_t bid_size;
        size_t ask_size;
};

class UpdateIdx {
    public:
        size_t t;
        size_t U_id;
        size_t u_id;
        size_t pu_id;
        size_t offset;
        size_t size;
        size_t bid_size;
        size_t ask_size;
};

ostream& operator<<(ostream& os, const SnapshotIdx& snapshot_idx);
ostream& operator<<(ostream& os, const UpdateIdx& update_idx);

class ObRawFiles {
    public:
        ObRawFiles();
        ~ObRawFiles();

        void import_snapshot(int file_id_from, int file_id_to=-1);
        void import_update(int file_id_from, int file_id_to=-1);

        unordered_map<string, Symbol*> symbols_map;
        string snapshot_raw_path;
        string update_raw_path;

};


};
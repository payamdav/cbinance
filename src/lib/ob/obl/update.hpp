#pragma once
#include <string>
#include <vector>
#include "snapshot.hpp"
#include <unordered_map>
#include <fstream>
#include <iostream>

using namespace std;

namespace obl {

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

ostream& operator<<(ostream& os, const UpdateIdx& update_idx);

class Update {
    public:
        string symbol;
        string binary_path;
        string index_path;

        ifstream update_idx;
        ifstream update_data;

        Update(string symbol);
        ~Update();

        void open();
        void close();

        bool is_sorted_by_t();
        bool is_sorted_by_u_id();
        bool is_pu_aligned();
        bool is_index_size_correct();
        bool is_offset_correct();

        size_t count();
        UpdateIdx get_index(size_t idx);
        size_t get_index_relevant_to_snapshot(SnapshotIdxLevel sidx, size_t start_index=0);
        void get_update(const UpdateIdx& update_idx, unordered_map<size_t, double>& b, unordered_map<size_t, double>& a);
};


};

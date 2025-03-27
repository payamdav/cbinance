#pragma once
#include "raw_files/ob_raw_files.hpp"
#include <string>
#include <vector>


using namespace std;


class Snapshot {
    public:
        string symbol;
        string binary_path;
        string index_path;

        ifstream snapshot_idx;
        ifstream snapshot_data;

        Snapshot(string symbol);
        ~Snapshot();

        void open();
        void close();

        bool is_sorted_by_t();
        bool is_sorted_by_u_id();

        size_t count();
        SnapshotIdx get_index(size_t idx);
        size_t get_index_gte(size_t t);
        void get_snapshot(const SnapshotIdx& snapshot_idx, vector<double>& bp, vector<double>& bv, vector<double>& ap, vector<double>& av);
};

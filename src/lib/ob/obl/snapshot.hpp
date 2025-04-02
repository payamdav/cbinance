#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>


using namespace std;

namespace obl {

class SnapshotIdxLevel {
    public:
        size_t t; // Time of the snapshot
        size_t u_id; // Unique ID of the snapshot
        size_t first_bid_level; // First level with non-zero volume for bids
        size_t last_bid_level; // Last level with non-zero volume for bids
        size_t first_ask_level; // First level with non-zero volume for asks
        size_t last_ask_level; // Last level with non-zero volume for asks
        size_t offset;
        size_t size;
};

ostream& operator<<(ostream& os, const SnapshotIdxLevel& snapshot_idx);



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
        SnapshotIdxLevel get_index(size_t idx);
        size_t get_index_gte(size_t t);
        void get_snapshot(const SnapshotIdxLevel& snapshot_idx, vector<double>& b, vector<double>& a);
};

};

#pragma once
#include "../snapshot.hpp"
#include <fstream>
#include <string>
#include "../../ta/pip_levelizer/pip_levelizer.hpp" // Include PipLevelizer if needed for conversion logic


using namespace std;


namespace ob {


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


class SnapshotConverter {
    public:
        string symbol;
        PipLevelizer levelizer; // Instance of PipLevelizer for levelizing prices, if needed
        Snapshot snapshot;
        ofstream snapshot_idx;
        ofstream snapshot_data;

        vector<double> b;
        vector<double> a;

        SnapshotConverter(string symbol);
        void convert_to_level_snapshot();
};


};
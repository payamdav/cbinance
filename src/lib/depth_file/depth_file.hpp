#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "../ta/pip_levelizer/pip_levelizer.hpp"

using namespace std;

struct Level {
    double price;
    double volume;
};


class DepthSnapshot {
    public:
        long long ts;
        vector<Level> bids;
        vector<Level> asks;
        PipLevelizer levelizer;
        vector<double> bids_volumes;
        vector<double> asks_volumes;
        size_t bids_min_level;
        size_t bids_max_level;
        size_t asks_min_level;
        size_t asks_max_level;

        DepthSnapshot(PipLevelizer levelizer);

        void parse_string(string line);
        void calculate_levelized_volumes();
        void write_levelized_to_file(string filename);
};


void process_file(string filename, PipLevelizer levelizer);


class Level_Depth {
    public:
        long long ts;
        size_t bids_min_level;
        size_t bids_max_level;
        size_t asks_min_level;
        size_t asks_max_level;
        vector<int> bids;
        vector<int> asks;

        Level_Depth();
        void parse_string(string line);
        int get_bids_volume(size_t level);
        int get_asks_volume(size_t level);
};

class Level_Depths : public vector<Level_Depth> {
    public:
        Level_Depths();
        void read_from_file(string filename);
        bool is_sorted();
        bool is_ts_exist(long long ts);
        size_t min_bids_level() const;
        size_t max_bids_level() const;
        size_t min_asks_level() const;
        size_t max_asks_level() const;

        Level_Depth get_level_depth(long long ts);
        Level_Depths get_level_depths(vector<long long> tss);
        Level_Depths get_level_depths(long long ts1, long long ts2, long long step=60000);


        Level_Depths filter(long long ts1, long long ts2);

        vector<int> bids_volumes();
        vector<int> asks_volumes();
        vector<int> volumes();

        int average_bids_volume();
        int average_asks_volume();
        int max_bids_volume();
        int max_asks_volume();
        

};


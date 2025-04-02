#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>
#include "../ob.hpp"
#include "../../ta/pip_levelizer/pip_levelizer.hpp"


using namespace std;

namespace ob {


class OBLC : public OB {
    public:
        unordered_map<size_t, double> diff_bids;
        unordered_map<size_t, double> diff_asks;
        ofstream update_idx;
        ofstream update_data;
        PipLevelizer levelizer; // Instance of PipLevelizer for levelizing prices
        OBLC(string symbol);
        void build(size_t from_ts=0, size_t to_ts=2000000000000) override;
        void apply_price_vol_update(const UpdateIdx & uidx) override;
        void write_diff_to_file(const UpdateIdx & uidx);
        void write_gap_update_to_file(const UpdateIdx & uidx);
};


};

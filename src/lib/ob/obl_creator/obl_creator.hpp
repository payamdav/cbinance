#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>
#include "../ob.hpp"
#include "../../ta/pip_levelizer/pip_levelizer.hpp"


using namespace std;


class OBLC : public OB {
    public:
        unordered_map<size_t, double> diff_bids;
        unordered_map<size_t, double> diff_asks;
        ofstream update_idx;
        ofstream update_data;
        PipLevelizer levelizer; // Instance of PipLevelizer for levelizing prices
        OBLC(string symbol);
        void apply_price_vol_update(const UpdateIdx & uidx) override;
        void write_diff_to_file(const UpdateIdx & uidx);
};

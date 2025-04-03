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


class OBLRec {
    public:
        size_t t;
        bool full;
        size_t offset;
        size_t bid_size;
        size_t ask_size;
};

const size_t OBL_REC_BYTES = sizeof(OBLRec);


class OBLB : public OB {
    public:
        ofstream obl_idx;
        ofstream obl_data;
        PipLevelizer levelizer; // Instance of PipLevelizer for levelizing prices
        size_t last_full_ts;
        vector<double> bidsl;
        vector<double> asksl;


        OBLB(string symbol);
        void apply_price_vol_snapshot(const SnapshotIdx & sidx) override;
        void apply_price_vol_update(const UpdateIdx & uidx) override;
};


};

#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "../../ta/pip_levelizer/pip_levelizer.hpp"
#include <fstream>

using namespace std;


namespace obl {

double eps = 0.00000001;


class OBLRec {
    public:
        size_t t;
        bool full;
        size_t offset;
        size_t bid_size;
        size_t ask_size;
};

const size_t OBL_REC_BYTES = sizeof(OBLRec);



class OBL {
    public:
        vector<double> bids;
        vector<double> asks;
        string symbol;
        size_t ucount;
        ifstream obl_idx;
        ifstream obl_data;
        size_t t;
        size_t idx; // current index in the order book
        size_t fb, lb, fa, la; // first and last levels for bids and asks

        OBL(string symbol);
        ~OBL();

        OBL * init(size_t from_ts=0);
        void apply_obl_rec(OBLRec &rec);
        OBL * next();
        bool ended();
        bool get_obl_rec(size_t idx, OBLRec &rec);
        size_t find_uid_full(size_t ts_to_go);

        void after_update();
        virtual void on_after_update() {
            // This can be overridden by subclasses to perform additional actions after each update
            // For example, logging or custom processing
        }

};

};

#pragma once
#include "snapshot.hpp"
#include "update.hpp"
#include <string>
#include <vector>
#include <iostream>
#include "../../ta/pip_levelizer/pip_levelizer.hpp"

using namespace std;


namespace obl {

class OB {
    protected:
        size_t scount;
        size_t ucount;
    public:
        vector<double> bids;
        vector<double> asks;
        string symbol;
        Snapshot* snapshot;
        Update* update;
        size_t u_id;
        size_t t;
        size_t idx; // current index in the order book

        size_t fb, lb, fa, la; // first and last levels for bids and asks

        OB(string symbol);
        ~OB();

        void build(size_t from_ts=0, size_t to_ts=2000000000000);
        bool find_sid_uid(size_t ts_to_go, size_t & sid, size_t & uid);
        void apply_snapshot(SnapshotIdxLevel & sidx);
        void apply_update(UpdateIdx & uidx);

        void after_update();
        virtual void on_after_update() {
            // This can be overridden by subclasses to perform additional actions after each update
            // For example, logging or custom processing
        }

};

};

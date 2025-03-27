#pragma once
#include "snapshot.hpp"
#include "update.hpp"
#include <string>
#include <vector>
#include <map>
#include <iostream>


using namespace std;

class OB {
    private:
        vector<double> bp;
        vector<double> bv;
        vector<double> ap;
        vector<double> av;
        size_t scount;
        size_t ucount;

        void clear_temp_vectores();
    public:
        map<double, double> bids;
        map<double, double> asks;
        string symbol;
        Snapshot* snapshot;
        Update* update;
        size_t u_id;
        size_t t;

        OB(string symbol);
        ~OB();

        void build(size_t from_ts=0, size_t to_ts=2000000000000);
        bool find_sid_uid(size_t ts_to_go, size_t & sid, size_t & uid);
        void apply_snapshot(SnapshotIdx & sidx);
        void apply_update(UpdateIdx & uidx);
        void apply_price_vol();

        void after_update();

};

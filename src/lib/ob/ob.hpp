#pragma once
#include "snapshot.hpp"
#include "update.hpp"
#include <string>
#include <vector>
#include <map>
#include <iostream>


using namespace std;

class OB {
    protected:
        vector<double> bp;
        vector<double> bv;
        vector<double> ap;
        vector<double> av;
        size_t scount;
        size_t ucount;
    public:
        map<double, double> bids;
        map<double, double> asks;
        string symbol;
        Snapshot* snapshot;
        Update* update;
        size_t u_id;
        size_t t;
        size_t idx; // current index in the order book

        OB(string symbol);
        ~OB();

        void build(size_t from_ts=0, size_t to_ts=2000000000000);
        bool find_sid_uid(size_t ts_to_go, size_t & sid, size_t & uid);
        void apply_snapshot(SnapshotIdx & sidx);
        void apply_update(UpdateIdx & uidx);
        virtual void apply_price_vol_snapshot(const SnapshotIdx &); // Apply price and volume to bids and asks
        virtual void apply_price_vol_update(const UpdateIdx &); // Apply price and volume to bids and asks

        void after_update();
        virtual void on_after_update() {
            // This can be overridden by subclasses to perform additional actions after each update
            // For example, logging or custom processing
        }

        void check();

};

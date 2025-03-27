#pragma once
#include "snapshot.hpp"
#include "update.hpp"
#include <string>
#include <vector>
#include <map>
#include <iostream>


using namespace std;

class OB {
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

};

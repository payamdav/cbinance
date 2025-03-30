#pragma once
#include <string>
#include <iostream>
#include <vector>

using namespace std;

class MarketInfo {
    public:
        string base_path;
        vector<string> symbols_list;

        MarketInfo();

        void set_min_max_price(const string &symbol, double min_price, double max_price);
        void get_min_max_price(const string &symbol, double &min_price, double &max_price);
};


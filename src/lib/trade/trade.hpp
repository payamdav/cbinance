#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

class Trade {
    public:
        double p;
        double v;
        double q;
        size_t t;
        bool is_buyer_maker;
};

ostream& operator<<(ostream& os, const Trade& trade);

class Trades : public vector<Trade> {
    private:
    public:
        string symbol;
        size_t count; // Number of trades in binary file
        ifstream trade_data;

        Trades(string symbol);
        void import_from_csv(int year, int month, int day);
        void open();
        void close();

        bool read_trade(size_t index, Trade &trade);
        Trade read_trade(size_t index);
        Trade read_first();
        Trade read_last();
        size_t search(size_t t);
        void read_by_index(size_t start, size_t num);
        void read_by_ts(size_t ts1, size_t ts2);
};

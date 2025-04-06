#pragma once
#include "trade.hpp"
#include "../ta/pip_levelizer/pip_levelizer.hpp"
#include <vector>
#include <fstream>
#include <iostream>


using namespace std;

class TL {
    public:
        size_t t; // start Timestamp of Trade Level
        size_t t_end; // End Timestamp of Trade Level
        size_t n; // Number of trades in this level
        size_t l; // Level of trades
        double v; // Volume of trades in this level
        double b; // Volume of aggressive buyers in this level
        double s; // Volume of aggressive sellers in this level

        TL();
        void add_trade(const Trade& trade); // Add a trade to this TL, updating n, v, b, s
};


ostream& operator<<(ostream& os, const TL& tl);

class TLS : public vector<TL> {
    public:
        string symbol;
        size_t count; // Number of TLs in this TLS (not used, but can be useful for debugging)
        ifstream tl_data; // File stream for reading TL data if needed


        TLS(string symbol);
        void open();
        void close();
        void set_file_cursor(size_t pos=0);
        void next(TL &tl);

        bool read(size_t index, TL &tl);
        size_t search(size_t t);
        void read_by_index(size_t start, size_t num);
        void read_by_ts(size_t ts1, size_t ts2);
        TL first_tl();
        TL last_tl();

        void add_trade(const Trade& trade, size_t l); // Add a trade to the appropriate TL based on the level
        void import_trades(Trades& trades);
        void save();
};


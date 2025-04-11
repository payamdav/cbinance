#pragma once
#include <string>
#include <vector>
#include "../ta/pip_levelizer/pip_levelizer.hpp"
#include <iostream>


using namespace std;


class BinanceCandle {
    public:
        long long ts;
        long long ts_close;
        double o;
        double h;
        double l;
        double c;
        double v;
        double qv;
        double vb;
        double vs;
        long n;
        size_t ol;
        size_t hl;
        size_t ll;
        size_t cl;

        BinanceCandle(string str, PipLevelizer * levelizer=nullptr);

        // bool from_str(std::string str);
        // void set_level(PipLevelizer & levelizer);
        void merge(BinanceCandle & candle);
};

ostream & operator<<(ostream & os, const BinanceCandle & candle);


class BinanceCandles : public std::vector<BinanceCandle> {
    public:
        string symbol;

        BinanceCandles(string symbol, size_t ts1=0, size_t ts2=0, PipLevelizer * levelizer=nullptr);
        void load_candles_csv(string csv_file ,PipLevelizer * levelizer=nullptr);
        // void set_levels(PipLevelizer & levelizer);
        bool is_sorted();
        int get_candles_interval() const;
        bool is_any_gap();
        BinanceCandles resample(int multiplier);

};

ostream & operator<<(ostream & os, const BinanceCandles & candles);
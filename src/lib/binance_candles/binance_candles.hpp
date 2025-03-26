#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <format>
#include "../config/config.hpp"
#include "../ta/pip_levelizer/pip_levelizer.hpp"


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

        void print();
        bool from_str(std::string str);
        void set_level(PipLevelizer & levelizer);
        void merge(BinanceCandle & candle);
};


class BinanceCandles : public std::vector<BinanceCandle> {
    public:
        void load_candles_csv(std::vector<std::string> csv_files);
        void set_levels(PipLevelizer & levelizer);
        bool is_sorted();
        int get_candles_interval();
        bool is_any_gap();
        BinanceCandles resample(int multiplier);

};

class BinanceCandlesFeeder : public BinanceCandles {
    public:
        BinanceCandlesFeeder();
        BinanceCandlesFeeder(BinanceCandles & candles);
        size_t last_feed_index;
        BinanceCandles feed_upto(long long ts);
};
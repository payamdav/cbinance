#pragma once
#include "trade.hpp"
#include <iostream>
#include "vector"
#include "string"
#include <fstream>
#include "../ta/pip_levelizer/pip_levelizer.hpp"



struct TradesStats {
    long long ts1;
    long long ts2;
    long long duration;
    double duration_hours;
    size_t min_level;
    size_t max_level;
    double min_volume;
    double max_volume;
    long long min_duration;
    long long max_duration;
    double total_volume;
    double avg_volume_per_hour;
    size_t trades_count;
};

class SingleTradesReader {
private:
    std::ifstream in;
    long long ts1;
    long long ts2;
public:
    SingleTradesReader(const std::string& file);
    SingleTradesReader(const std::string& file, long long ts1, long long ts2);
    SingleTrade next();
    void close();
};


class Trades : public std::vector<Trade> {
public:
    void load_from_csv(const std::string& file);
    void save_binary(const std::string& file);
    void load_binary(const std::string& file);
    void load_binary(const std::string& file, long long ts1, long long ts2);
    void save_for_gnuplot(const std::string& file);
    void save_for_gnuplot_w_vol_merger(const std::string& file, const std::vector<double>& vols);
    void append_single_trade(const SingleTrade& single_trade, size_t l);
    void append_single_trade(const SingleTrade& single_trade, PipLevelizer& levelizer);
    void stats_print();
    TradesStats stats();
};

std::ostream& operator<<(std::ostream& os, const Trades& trades);


class TradesReader {
private:
    std::ifstream in;
    long long ts1;
    long long ts2;
public:
    TradesReader(const std::string& file);
    TradesReader(const std::string& file, long long ts1, long long ts2);
    Trade next();
    void close();
};

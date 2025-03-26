#pragma once
#include <string>
#include "../ta/pip_levelizer/pip_levelizer.hpp"

class TradeOriginal {
public:
    long long ts;
    double p;
    double v;
    double q;
    bool is_buyer_maker;
    size_t l;

    TradeOriginal(): ts(0), p(0), v(0), q(0), is_buyer_maker(false), l(0) {};
    TradeOriginal(const std::string & line);
    size_t set_level(PipLevelizer & levelizer);

};

class SingleTrade {
public:
    long long ts;
    double v;
    double q;
    bool is_buyer_maker;

    SingleTrade(): ts(0), v(0), q(0), is_buyer_maker(false) {};
};


class Trade {
public:
    long long ts;
    long long ts_last;
    double v;
    double q;
    double vs;
    double vb;
    double qs;
    double qb;
    size_t l; // level
    long long duration;

    Trade(): ts(0), ts_last(0), v(0), q(0), vs(0), vb(0), qs(0), qb(0), l(0), duration(0) {};
    
    void append_trade_original(const TradeOriginal & trade_original);

};


std::ostream& operator<<(std::ostream& os, const Trade& trade);
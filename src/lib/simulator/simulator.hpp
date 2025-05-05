#pragma once
#include "../trade/trade.hpp"
#include <string>

using namespace std;

class Simulator {
    public:
        string symbol;
        size_t start_ts;
        size_t end_ts;

        Simulator(string symbol, size_t start_ts=0, size_t end_ts=1800000000000);
        virtual void pre();
        Simulator * run();
        virtual void new_trade(Trade &trade);
        virtual void post();
};



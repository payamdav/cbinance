#pragma once
#include "../timeseries/timeseries.hpp"
#include <vector>
#include "../../binance_candles/binance_candles.hpp"
#include <iostream>


class Vols : public TimeSeries {
    public:
        vector<double> v;
        vector<double> b;
        vector<double> s;

        Vols(BinanceCandles &candles, int n=1); // n is the number of previus samples to get average with
};

        

#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "../../binance_candles/binance_candles.hpp"
#include "../../ta/pip_levelizer/pip_levelizer.hpp"

using namespace std;

struct VolumeArea {
    size_t ts_center;
    size_t ts_start;
    size_t ts_end;
    size_t level_center;
    double avg_volume;
};

class CandleAnal {
public:
    BinanceCandles * candles;
    PipLevelizer * levelizer;
    vector<size_t> vwl;

    CandleAnal(string symbol, size_t ts1, size_t ts2);

    double average_candle_size();
    void save_vwl(string filename);

    vector<double> & volume_convolution(int period, int maxlevel=10000);
    double average_volume();
    vector<VolumeArea> volume_areas(int period, int maxlevel=10000);

};

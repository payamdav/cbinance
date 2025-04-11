#include "binance_candles.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <format>
#include "../utils/datetime_utils.hpp"
#include "../utils/string_utils.hpp"
#include "../config/config.hpp"


using namespace std;

// BinanceCandle

BinanceCandle::BinanceCandle(string str, PipLevelizer * levelizer) {
    int result = sscanf(str.c_str(), "%lld,%lf,%lf,%lf,%lf,%lf,%lld,%lf,%ld,%lf,%*f,%*d", &ts, &o, &h, &l, &c, &v, &ts_close, &qv, &n, &vb);
    if (result != 10) {
        std::cout << "Error parsing candle: " << str << std::endl;
        ts = 0;
    }
    vs = v - vb;
    ol = hl = ll = cl = 0;
    if (levelizer) {
        ol = (*levelizer)(o);
        hl = (*levelizer)(h);
        ll = (*levelizer)(l);
        cl = (*levelizer)(c);
    }
}


void BinanceCandle::merge(BinanceCandle & candle) {
    if (candle.ts <= ts) {
        std::cout << "Error merging candles: Candle ts is less than or equal to ts" << std::endl;
        return;
    }
    ts_close = candle.ts;
    c = candle.c;
    if (candle.h > h) h = candle.h;
    if (candle.l < l) l = candle.l;
    v += candle.v;
    qv += candle.qv;
    vb += candle.vb;
    vs += candle.vs;
    n += candle.n;
    cl = candle.cl;
    if (candle.hl > hl) hl = candle.hl;
    if (candle.ll < ll) ll = candle.ll;
}

ostream & operator<<(ostream & os, const BinanceCandle & candle) {
    os << format("ts: {}, ts_close: {}, o: {}, h: {}, l: {}, c: {}, v: {}, qv: {}, vb: {}, vs: {}, n: {}, ol: {}, hl: {}, ll: {}, cl: {}",
        candle.ts, candle.ts_close, candle.o, candle.h, candle.l, candle.c, candle.v, candle.qv, candle.vb, candle.vs, candle.n, candle.ol, candle.hl, candle.ll, candle.cl);
    return os;
}


// BinanceCandles

BinanceCandles::BinanceCandles(string symbol, size_t ts1, size_t ts2, PipLevelizer * levelizer) : symbol(utils::toLowerCase(symbol)) {
    if (ts1 != 0 && ts2 != 0) {
        string base_path = config.get_path("data_path") + "um/candles/" + utils::toUpperCase(symbol) + "-1m-";
        auto year_month_days = utils::get_year_month_days(ts1, ts2);
        for (auto & ymd : year_month_days) {
            string csv_file = base_path + to_string(ymd.year) + "-" + utils::lpad(to_string(ymd.month), '0', 2) + "-" + utils::lpad(to_string(ymd.day), '0', 2) + ".csv";
            load_candles_csv(csv_file, levelizer);
        }
    }
}

void BinanceCandles::load_candles_csv(string csv_file, PipLevelizer * levelizer) {
        cout << "Loading " << csv_file << endl;
        ifstream file(csv_file);
        string line;
        while (getline(file, line)) {
            if (line[0] == '1') {
                this->emplace_back(line, levelizer);
                if (back().ts == 0) this->pop_back();
            }            
        }
}

bool BinanceCandles::is_sorted() {
    for (size_t i = 1; i < this->size(); i++) {
        if ((*this)[i].ts < (*this)[i-1].ts) {
            return false;
        }
    }
    return true;
}

int BinanceCandles::get_candles_interval() const {
    if (this->size() < 2) {
        return 0;
    }
    return (*this)[1].ts - (*this)[0].ts;
}

bool BinanceCandles::is_any_gap() {
    int interval = get_candles_interval();
    for (size_t i = 1; i < this->size(); i++) {
        if ((*this)[i].ts - (*this)[i-1].ts != interval) {
            return true;
        }
    }
    return false;
}

BinanceCandles BinanceCandles::resample(int multiplier) {
    BinanceCandles resampled(symbol);
    for (size_t i = 0; i < this->size(); i++) {
        if (i % multiplier == 0) {
            // copy the candle and push it to resampled
            resampled.push_back((*this)[i]);
        } else {
            // merge the candle with the last candle in resampled
            resampled.back().merge((*this)[i]);
        }
    }
    BinanceCandle last_candle = resampled.back();
    if (last_candle.ts_close - last_candle.ts + 1 != get_candles_interval() * multiplier) {
        // remove the last candle
        resampled.pop_back();
    }
    return resampled;
}

ostream & operator<<(ostream & os, const BinanceCandles & candles) {
    os << format("symbol: {}, size: {}, interval: {}", candles.symbol, candles.size(), candles.get_candles_interval());
    return os;
}

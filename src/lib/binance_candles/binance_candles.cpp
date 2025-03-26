#include "binance_candles.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <format>

void BinanceCandle::print() {
    std::cout << std::format("ts: {}, ts_close: {}, o: {}, h: {}, l: {}, c: {}, v: {}, qv: {}, vb: {}, vs: {}, n: {}, ol: {}, hl: {}, ll: {}, cl: {}", ts, ts_close, o, h, l, c, v, qv, vb, vs, n, ol, hl, ll, cl) << std::endl;
}

bool BinanceCandle::from_str(std::string str) {
    int result = sscanf(str.c_str(), "%lld,%lf,%lf,%lf,%lf,%lf,%lld,%lf,%ld,%lf,%*f,%*d", &ts, &o, &h, &l, &c, &v, &ts_close, &qv, &n, &vb);
    if (result != 10) {
        std::cout << "Error parsing candle: " << str << std::endl;
        return false;
    }
    vs = v - vb;
    ol = hl = ll = cl = 0;
    return true;
}

void BinanceCandle::set_level(PipLevelizer & levelizer) {
    ol = levelizer.get_level(o);
    hl = levelizer.get_level(h);
    ll = levelizer.get_level(l);
    cl = levelizer.get_level(c);
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


// BinanceCandles

void BinanceCandles::load_candles_csv(std::vector<std::string> csv_files) {
    for (auto csv_file : csv_files) {
        std::cout << "Loading " << csv_file << std::endl;
        std::ifstream file(csv_file);
        std::string line;
        // ignor the first line
        // std::getline(file, line);
        while (std::getline(file, line)) {
            BinanceCandle candle;
            if (candle.from_str(line)) this->push_back(candle);
        }
        file.close();
    }
}

void BinanceCandles::set_levels(PipLevelizer & levelizer) {
    for (auto & candle : *this) {
        candle.set_level(levelizer);
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

int BinanceCandles::get_candles_interval() {
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
    BinanceCandles resampled;
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


// BinanceCandlesFeeder

BinanceCandlesFeeder::BinanceCandlesFeeder() {
    last_feed_index = -1;
}

BinanceCandlesFeeder::BinanceCandlesFeeder(BinanceCandles & candles) {
    for (auto & candle : candles) {
        this->push_back(candle);
    }
    last_feed_index = -1;
}

BinanceCandles BinanceCandlesFeeder::feed_upto(long long ts) {
    BinanceCandles candles;
    for (size_t i = last_feed_index + 1; i < this->size(); i++) {
        if ((*this)[i].ts <= ts) {
            candles.push_back((*this)[i]);
            last_feed_index = i;
        } else {
            break;
        }
    }
    return candles;
}
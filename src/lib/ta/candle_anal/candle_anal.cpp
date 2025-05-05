#include "candle_anal.hpp"
#include "../../utils/datetime_utils.hpp"
#include "../../utils/string_utils.hpp"
#include "../../ta/pip_levelizer/pip_levelizer.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <ranges>


using namespace std;

CandleAnal::CandleAnal(string symbol, size_t ts1, size_t ts2) {
    levelizer = new PipLevelizer(symbol);
    candles = new BinanceCandles(symbol, ts1, ts2, levelizer);
    // filter candles to the range
    candles->erase(remove_if(candles->begin(), candles->end(), [ts1, ts2](const BinanceCandle & candle) {
        return candle.ts < ts1 || candle.ts > ts2;
    }), candles->end());
    // cout << "Filtered candles: " << *candles << endl;
    // cout << "Average candle size: " << average_candle_size() << endl;
    vwl.reserve(candles->size());
    for (const auto & candle : *candles) {
        vwl.emplace_back((*levelizer)(candle.qv/candle.v));
    }

    // cout << "vwl max: " << *max_element(vwl.begin(), vwl.end()) << endl;
    // cout << "vwl min: " << *min_element(vwl.begin(), vwl.end()) << endl;
}


double CandleAnal::average_candle_size() {
    double total_size = 0;
    for (const auto & candle : *candles) {
        total_size += (candle.hl - candle.ll);
    }
    return total_size / candles->size();
}

void CandleAnal::save_vwl(string filename) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }
    for (int i = 0; i < vwl.size(); i++) {
        file.write(reinterpret_cast<const char *>(&(candles->at(i).ts)), sizeof(size_t));
        file.write(reinterpret_cast<const char *>(&vwl[i]), sizeof(size_t));
    }
    file.close();
}


vector<double> & CandleAnal::volume_convolution(int period, int maxlevel) {
    vector<double> * result = new vector<double>(candles->size(), 0);
    int center = period / 2;
    for (int i = 0; i < candles->size(); i++) {
        double sum = 0;
        for (int j = -center; j <= center; j++) {
            if (i + j < 0 || i + j >= candles->size()) continue;
            if (abs((int)(vwl[i + j] - vwl[i])) > maxlevel) continue;
            sum += candles->at(i + j).v;
        }
        result->at(i) = sum / period;
    }
    return *result;
}

double CandleAnal::average_volume() {
    double total_volume = 0;
    for (const auto & candle : *candles) {
        total_volume += candle.v;
    }
    return total_volume / candles->size();
}

vector<VolumeArea> CandleAnal::volume_areas(int period, int maxlevel) {
    vector<VolumeArea> areas;
    double avg_volume = average_volume();
    vector<double> & conv = volume_convolution(period, maxlevel);

    size_t count = 0;

    while(true) {
        auto it_max = max_element(conv.begin(), conv.end());
        double max_conv = *it_max;
        size_t max_index = distance(conv.begin(), it_max);
        if (max_conv < avg_volume) break;
        VolumeArea area;
        area.rank = count++;
        area.max_level = maxlevel;
        area.ts_center = candles->at(max_index).ts;
        area.level_center = vwl[max_index];
        area.avg_volume = 0;
        size_t start_index=max_index; 
        size_t end_index=max_index;
        // finding start
        for (; start_index >= 0; start_index--) {
            if (conv[start_index] < avg_volume) {
                start_index++;
                break;
            }
        }
        // finding end
        for (; end_index < conv.size(); end_index++) {
            if (conv[end_index] < avg_volume) {
                end_index--;
                break;
            }
        }
        area.ts_start = candles->at(start_index).ts;
        area.ts_end = candles->at(end_index).ts;
        area.avg_volume = 0;
        // calculate average volume
        for (int i = start_index; i <= end_index; i++) {
            area.avg_volume += candles->at(i).v;
        }

        area.avg_volume /= (end_index - start_index + 1);
        areas.push_back(area);
        // zero out the area
        for (int i = start_index; i <= end_index; i++) {
            conv[i] = 0;
        }
    }

    return areas;
}

void CandleAnal::save_volume_areas(const vector<VolumeArea> & areas, const string & filename) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }
    for (const auto & area : areas) {
        file.write(reinterpret_cast<const char *>(&area.rank), sizeof(size_t));
        file.write(reinterpret_cast<const char *>(&area.ts_center), sizeof(size_t));
        file.write(reinterpret_cast<const char *>(&area.ts_start), sizeof(size_t));
        file.write(reinterpret_cast<const char *>(&area.ts_end), sizeof(size_t));
        file.write(reinterpret_cast<const char *>(&area.level_center), sizeof(size_t));
        file.write(reinterpret_cast<const char *>(&area.max_level), sizeof(size_t));
        file.write(reinterpret_cast<const char *>(&area.avg_volume), sizeof(double));
    }
    file.close();
}


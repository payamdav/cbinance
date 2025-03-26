#include "trades.hpp"
#include <fstream>
#include "../config/config.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>
#include "../utils/datetime_utils.hpp"



SingleTradesReader::SingleTradesReader(const std::string& file, long long ts1, long long ts2) : ts1(ts1), ts2(ts2), in(file, std::ios::binary) {
    if (!in.is_open()) {
        std::cerr << "Error opening file: " << file << std::endl;
        return;
    }
    if (ts1 == 0) {
        return;
    }
    in.seekg(0, std::ios::end);
    size_t size = in.tellg() / sizeof(SingleTrade);
    in.seekg(0, std::ios::beg);
    size_t left = 0;
    size_t right = size;
    while (left < right) {
        size_t mid = (left + right) / 2;
        in.seekg(mid * sizeof(SingleTrade));
        SingleTrade single_trade;
        in.read((char*)&single_trade, sizeof(SingleTrade));
        if (single_trade.ts == ts1) {
            left = mid;
            break;
        } else if (single_trade.ts < ts1) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    in.seekg(left * sizeof(SingleTrade));
}

SingleTradesReader::SingleTradesReader(const std::string& file) : SingleTradesReader(file, 0, 0) {}

SingleTrade SingleTradesReader::next() {
    SingleTrade single_trade;
    in.read((char*)&single_trade, sizeof(SingleTrade));
    if (in.eof() || (ts2 > 0 && single_trade.ts > ts2)) {
        single_trade.ts = 0;
    }
    return single_trade;
}

void SingleTradesReader::close() {
    in.close();
}


void Trades::load_from_csv(const std::string& csv_file) {
    long long trade_counter = 0;
    this->clear();
    PipLevelizer levelizer;
    std::ifstream file(csv_file);
    size_t last_level = 0;
    std::string line;
    std::getline(file, line); // skip header
    while (std::getline(file, line)) {
        trade_counter++;
        if (trade_counter % 1000000 == 0) {
            std::cout << "Trade: " << trade_counter << std::endl;
        }
        TradeOriginal trade(line);
        trade.set_level(levelizer);
        if (last_level != trade.l) {
            last_level = trade.l;
            this->emplace_back();
        }
        this->back().append_trade_original(trade);
    }
}

void Trades::save_binary(const std::string& file) {
    std::ofstream out(file, std::ios::binary | std::ios::out | std::ios::app);
    out.seekp(0, std::ios::end);
    for (size_t i = 0; i < this->size(); i++) {
        out.write((char*)&this->at(i), sizeof(Trade));
    }
    out.close();
}

void Trades::load_binary(const std::string& file) {
    this->clear();
    std::ifstream in(file, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Error opening file: " << file << std::endl;
        return;
    }
    while (true) {
        Trade trade;
        in.read((char*)&trade, sizeof(Trade));
        if (in.eof()) {
            break;
        }
        this->push_back(trade);
    }
    in.close();
}

void Trades::load_binary(const std::string& file, long long ts1, long long ts2) {
    this->clear();
    std::ifstream in(file, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Error opening file: " << file << std::endl;
        return;
    }
    in.seekg(0, std::ios::end);
    size_t size = in.tellg() / sizeof(Trade);
    in.seekg(0, std::ios::beg);
    size_t left = 0;
    size_t right = size;
    while (left < right) {
        size_t mid = (left + right) / 2;
        in.seekg(mid * sizeof(Trade));
        Trade trade;
        in.read((char*)&trade, sizeof(Trade));
        if (trade.ts == ts1) {
            left = mid;
            break;
        } else if (trade.ts < ts1) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    in.seekg(left * sizeof(Trade));
    while (true) {
        Trade trade;
        in.read((char*)&trade, sizeof(Trade));
        if (in.eof() || trade.ts > ts2) {
            break;
        }
        this->push_back(trade);
    }
    in.close();
}


std::ostream& operator<<(std::ostream& os, const Trades& trades) {
    os << "Trades -> size: " << trades.size() << " First ts: " << trades[0].ts << " Last ts: " << trades[trades.size()-1].ts << " First Date: " << utils::get_utc_datetime_string(trades[0].ts) << " Last Date: " << utils::get_utc_datetime_string(trades[trades.size()-1].ts) << std::endl;
    return os;
}


// std::vector<std::string> Trades::to_string_space_separated() {
//     vector<string> result;
//     for (size_t i = 0; i < this->size(); i++) {
//         result.push_back(this->at(i).to_string_space_separated());
//     }
//     return move(result);
// }

void Trades::stats_print() {
    auto stats = this->stats();
    cout << "Trades Stats:" << endl;
    cout << "ts1: " << stats.ts1 << " ts2: " << stats.ts2 << " duration: " << stats.duration << " duration_hours: " << stats.duration_hours << endl;
    cout << "min_volume: " << stats.min_volume << " max_volume: " << stats.max_volume << " total_volume: " << stats.total_volume << " avg_volume_per_hour: " << stats.avg_volume_per_hour << endl;
    cout << "min_level: " << stats.min_level << " max_level: " << stats.max_level << endl;
    cout << "min_duration: " << stats.min_duration << " max_duration: " << stats.max_duration << endl;
    cout << "trades_count: " << stats.trades_count << endl;
    
}

TradesStats Trades::stats() {
    TradesStats stats;
    stats.ts1 = this->at(0).ts;
    stats.ts2 = this->at(this->size()-1).ts;
    stats.duration = stats.ts2 - stats.ts1;
    stats.duration_hours = stats.duration / 3600000.0;
    stats.min_level = this->at(0).l;
    stats.max_level = this->at(0).l;
    stats.min_volume = this->at(0).v;
    stats.max_volume = this->at(0).v;
    stats.min_duration = this->at(0).duration;
    stats.max_duration = this->at(0).duration;
    stats.total_volume = 0;
    for (size_t i = 0; i < this->size(); i++) {
        stats.total_volume += this->at(i).v;
        if (this->at(i).v < stats.min_volume) {
            stats.min_volume = this->at(i).v;
        }
        if (this->at(i).v > stats.max_volume) {
            stats.max_volume = this->at(i).v;
        }
        if (this->at(i).l < stats.min_level) {
            stats.min_level = this->at(i).l;
        }
        if (this->at(i).l > stats.max_level) {
            stats.max_level = this->at(i).l;
        }
        if (this->at(i).duration < stats.min_duration) {
            stats.min_duration = this->at(i).duration;
        }
        if (this->at(i).duration > stats.max_duration) {
            stats.max_duration = this->at(i).duration;
        }
    }
    stats.avg_volume_per_hour = stats.total_volume / stats.duration_hours;
    stats.trades_count = this->size();
    return stats;
}


TradesReader::TradesReader(const std::string& file, long long ts1, long long ts2) : ts1(ts1), ts2(ts2), in(file, std::ios::binary) {
    if (!in.is_open()) {
        std::cerr << "Error opening file: " << file << std::endl;
        return;
    }
    if (ts1 == 0) {
        return;
    }
    in.seekg(0, std::ios::end);
    size_t size = in.tellg() / sizeof(Trade);
    in.seekg(0, std::ios::beg);
    size_t left = 0;
    size_t right = size;
    while (left < right) {
        size_t mid = (left + right) / 2;
        in.seekg(mid * sizeof(Trade));
        Trade trade;
        in.read((char*)&trade, sizeof(Trade));
        if (trade.ts == ts1) {
            left = mid;
            break;
        } else if (trade.ts < ts1) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    in.seekg(left * sizeof(Trade));
}

TradesReader::TradesReader(const std::string& file) : TradesReader(file, 0, 0) {}

Trade TradesReader::next() {
    Trade trade;
    in.read((char*)&trade, sizeof(Trade));
    if (in.eof() || (ts2 > 0 && trade.ts > ts2)) {
        trade.ts = 0;
    }
    return trade;
}

void TradesReader::close() {
    in.close();
}

void Trades::append_single_trade(const SingleTrade& single_trade, size_t l) {
    if (this->empty() || this->back().l != l) {
        this->emplace_back();
        this->back().l = l;
        this->back().ts = single_trade.ts;
    }
    this->back().ts_last = single_trade.ts;
    this->back().v += single_trade.v;
    this->back().q += single_trade.q;
    if (single_trade.is_buyer_maker) {
        this->back().vs += single_trade.v;
        this->back().qs += single_trade.q;
    } else {
        this->back().vb += single_trade.v;
        this->back().qb += single_trade.q;
    }
    this->back().duration = this->back().ts_last - this->back().ts;

}

void Trades::append_single_trade(const SingleTrade& single_trade, PipLevelizer& levelizer) {
    size_t l = levelizer.get_level_binary_search(single_trade.q / single_trade.v);
    append_single_trade(single_trade, l);
}

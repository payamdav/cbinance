#include "obl_instant.hpp"
#include <iostream>
#include "../../../config/config.hpp"
#include <fstream>


const double eps = 0.00000001;

void obl_cache_to_file(string symbol, size_t ts1, size_t ts2, string file_name, size_t level_count) {
    obl::OBL obl(symbol);
    obl.init(ts1);
    while(obl.t < ts1) {
        obl.next();
    }

    size_t count = 0;
    size_t bid_max_level = 0;
    size_t bid_min_level = obl.bids.size();
    size_t ask_max_level = 0;
    size_t ask_min_level = obl.bids.size();
    double bid_max = 0;
    double ask_max = 0;
    double bid_count = 0;
    double ask_count = 0;
    double bid_sum = 0;
    double ask_sum = 0;
    double bid_avg = 0;
    double ask_avg = 0;
    size_t prev_ts = obl.t;
    size_t max_ts_distance = 0;


    string file_path = config.get_path("data_path") + "files/" + file_name;
    ofstream file(file_path, ios::binary);
    // file structure -> count(size_t), count_levels(size_t), bid_max(double), ask_max(double), bid_avg(double), ask_avg(double),
    // records: ts1(size_t), bids_start_level(size_t), count_levels * bids(double), asks_start_level(size_t), count_levels * asks(double)

    // seek to first record
    file.seekp( 6 * 8, ios::beg);


    while(!obl.ended() && obl.t <= ts2) {
        count++;

        if (obl.fb < bid_min_level) bid_min_level = obl.fb;
        if (obl.lb > bid_max_level) bid_max_level = obl.lb;
        if (obl.fa < ask_min_level) ask_min_level = obl.fa;
        if (obl.la > ask_max_level) ask_max_level = obl.la;
        for (size_t i = obl.fb; i <= obl.lb; i++) {
            if (obl.bids[i] > bid_max) bid_max = obl.bids[i];
            if (obl.bids[i] > eps) {
                bid_count++;
                bid_sum += obl.bids[i];

            }
        }
        for (size_t i = obl.fa; i <= obl.la; i++) {
            if (obl.asks[i] > ask_max) ask_max = obl.asks[i];
            if (obl.asks[i] > eps) {
                ask_count++;
                ask_sum += obl.asks[i];
            }
        }

        if (obl.t - prev_ts > max_ts_distance) {
            max_ts_distance = obl.t - prev_ts;
        }
        prev_ts = obl.t;

        // writing record to file
        file.write(reinterpret_cast<const char*>(&obl.t), sizeof(size_t));
        file.write(reinterpret_cast<const char*>(&obl.lb), sizeof(size_t));
        for (size_t i = obl.lb; i > obl.lb - level_count; i--) {
            double b = i < obl.bids.size() ? obl.bids[i] : 0;
            file.write(reinterpret_cast<const char*>(&b), sizeof(double));
        }
        file.write(reinterpret_cast<const char*>(&obl.fa), sizeof(size_t));
        for (size_t i = obl.fa; i < obl.fa + level_count; i++) {
            double a = i < obl.asks.size() ? obl.asks[i] : 0;
            file.write(reinterpret_cast<const char*>(&a), sizeof(double));
        }

        obl.next();
    }
    if (bid_count > 0) bid_avg = bid_sum / bid_count;
    if (ask_count > 0) ask_avg = ask_sum / ask_count;

    file.seekp(0, ios::beg);
    file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(&level_count), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(&bid_max), sizeof(double));
    file.write(reinterpret_cast<const char*>(&ask_max), sizeof(double));
    file.write(reinterpret_cast<const char*>(&bid_avg), sizeof(double));
    file.write(reinterpret_cast<const char*>(&ask_avg), sizeof(double));
    file.close();

    cout << "OBL statistics for " << symbol << " from " << ts1 << " to " << ts2 << endl;
    cout << "Count: " << count << endl;
    cout << "Bid levels: " << bid_min_level << " to " << bid_max_level << endl;
    cout << "Ask levels: " << ask_min_level << " to " << ask_max_level << endl;
    cout << "Bid max: " << bid_max << endl;
    cout << "Ask max: " << ask_max << endl;
    cout << "Bid avg: " << bid_avg << endl;
    cout << "Ask avg: " << ask_avg << endl;
    cout << "Max ts distance: " << max_ts_distance << endl;



}
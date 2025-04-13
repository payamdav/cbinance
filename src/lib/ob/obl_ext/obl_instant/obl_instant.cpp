#include "obl_instant.hpp"
#include <iostream>
#include "../../../config/config.hpp"
#include <fstream>
#include <cmath>
#include <map>
#include "../../../ta/vols/vols.hpp"
#include "../../../binance_candles/binance_candles.hpp"


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

vector<OBL_INSTANT_REC> obl_cache(string symbol, size_t ts1, size_t ts2, size_t level_count) {
    vector<OBL_INSTANT_REC> records;

    obl::OBL obl(symbol);
    obl.init(ts1);
    while(obl.t < ts1) {
        obl.next();
    }

    while(!obl.ended() && obl.t <= ts2) {
        OBL_INSTANT_REC rec;
        rec.t = obl.t;
        rec.bids_start_level = obl.lb;
        rec.asks_start_level = obl.fa;
        for (size_t i = obl.lb; i > obl.lb - level_count; i--) {
            double b = i < obl.bids.size() ? obl.bids[i] : 0;
            rec.bids.push_back(b);
        }
        for (size_t i = obl.fa; i < obl.fa + level_count; i++) {
            double a = i < obl.asks.size() ? obl.asks[i] : 0;
            rec.asks.push_back(a);
        }
        records.push_back(rec);
        obl.next();
    }
    return records;
}


vector<OBL_INSTANT_SCORE_REC> obl_cache_score(vector<OBL_INSTANT_REC> & records) {
    vector<OBL_INSTANT_SCORE_REC> rs;

    size_t count = 0;
    size_t count_bids = 0;
    size_t count_asks = 0;
    double sum_bids = 0;
    double sum_asks = 0;
    double avg_bids = 0;
    double avg_asks = 0;
    double std_bids = 0;
    double std_asks = 0;

    for (auto & rec : records) {
        count++;
        for (auto & b : rec.bids) {
            if (b > eps) {
                count_bids++;
                sum_bids += b;
            }
        }
        for (auto & a : rec.asks) {
            if (a > eps) {
                count_asks++;
                sum_asks += a;
            }
        }
    }

    if (count_bids > 0) avg_bids = sum_bids / count_bids;
    if (count_asks > 0) avg_asks = sum_asks / count_asks;

    // calculate std

    for (auto & rec : records) {
        for (auto & b : rec.bids) {
            if (b > eps) {
                std_bids += pow(b - avg_bids, 2);
            }
        }
        for (auto & a : rec.asks) {
            if (a > eps) {
                std_asks += pow(a - avg_asks, 2);
            }
        }
    }

    if (count_bids > 0) std_bids = sqrt(std_bids / count_bids);
    if (count_asks > 0) std_asks = sqrt(std_asks / count_asks);

    double score_factor_bids = std_bids / 1;
    double score_factor_asks = std_asks / 1;

    // building result with z-scores
    for (auto & rec : records) {
        OBL_INSTANT_SCORE_REC r;
        r.t = rec.t;
        r.bids_start_level = rec.bids_start_level;
        r.asks_start_level = rec.asks_start_level;
        for (auto & b : rec.bids) {
            r.bids.push_back(round((b - avg_bids) / score_factor_bids));
        }
        for (auto & a : rec.asks) {
            r.asks.push_back(round((a - avg_asks) / score_factor_asks));
        }
        rs.push_back(r);
    }

    map<int, int> bid_hist;
    map<int, int> ask_hist;
    for (auto & rec : rs) {
        for (auto & b : rec.bids) {
            if (bid_hist.find(b) == bid_hist.end()) {
                bid_hist[b] = 1;
            }
            else {
                bid_hist[b]++;
            }
        }
        for (auto & a : rec.asks) {
            if (ask_hist.find(a) == ask_hist.end()) {
                ask_hist[a] = 1;
            }
            else {
                ask_hist[a]++;
            }
        }
    }

    cout << "Bid histogram:" << endl;
    for (auto & b : bid_hist) {
        cout << b.first << ": " << b.second << endl;
    }
    cout << "Ask histogram:" << endl;
    for (auto & a : ask_hist) {
        cout << a.first << ": " << a.second << endl;
    }


    return rs;
}


vector<OBL_INSTANT_REC> obl_cache_ratio_to_vols(string symbol, size_t ts1, size_t ts2, size_t level_count, size_t vols_n) {
    vector<OBL_INSTANT_REC> records;
    BinanceCandles candles(symbol, ts1 - ((vols_n + 10) * 60000), ts2 + 60000); // get candles for 10 minutes before and after
    Vols vols(candles, vols_n);

    obl::OBL obl(symbol);
    obl.init(ts1);
    while(obl.t < ts1) {
        obl.next();
    }

    while(!obl.ended() && obl.t <= ts2) {
        OBL_INSTANT_REC rec;
        vols.forward_search_le(obl.t);
        rec.t = obl.t;
        rec.bids_start_level = obl.lb;
        rec.asks_start_level = obl.fa;
        for (size_t i = obl.lb; i > obl.lb - level_count; i--) {
            double b = i < obl.bids.size() ? obl.bids[i] / vols.s[vols.idx] : 0;
            rec.bids.push_back(b);
        }
        for (size_t i = obl.fa; i < obl.fa + level_count; i++) {
            double a = i < obl.asks.size() ? obl.asks[i] / vols.b[vols.idx] : 0;
            rec.asks.push_back(a);
        }
        records.push_back(rec);
        obl.next();
    }


    return records;
}


vector<OBL_INSTANT_AVG_REC> obl_cache_ratio_to_vols_avg(string symbol, size_t ts1, size_t ts2, size_t level_count, size_t vols_n, string file_name) {
    vector<OBL_INSTANT_AVG_REC> records;
    BinanceCandles candles(symbol, ts1 - ((vols_n + 10) * 60000), ts2 + 60000); // get candles for 10 minutes before and after
    Vols vols(candles, vols_n);

    obl::OBL obl(symbol);
    obl.init(ts1);
    while(obl.t < ts1) {
        obl.next();
    }

    while(!obl.ended() && obl.t <= ts2) {
        OBL_INSTANT_AVG_REC rec;
        vols.forward_search_le(obl.t);
        rec.t = obl.t;
        rec.b = 0;
        rec.a = 0;
        for (size_t i = obl.lb; i > obl.lb - level_count; i--) {
            double b = i < obl.bids.size() ? obl.bids[i] / vols.s[vols.idx] : 0;
            rec.b += b;
        }
        rec.b /= level_count;
        for (size_t i = obl.fa; i < obl.fa + level_count; i++) {
            double a = i < obl.asks.size() ? obl.asks[i] / vols.b[vols.idx] : 0;
            rec.a += a;
        }
        rec.a /= level_count;
        records.push_back(rec);
        obl.next();
    }

    if (file_name != "") {
        ofstream file(config.get_path("data_path") + "files/" + file_name, ios::binary);
        file.write(reinterpret_cast<const char*>(records.data()), sizeof(OBL_INSTANT_AVG_REC) * records.size());
    }
    return records;
}


void obl_limited_snapshots_to_file(string symbol, size_t ts1, size_t ts2, size_t l1, size_t l2, size_t interval, string file_name) {
    obl::OBL obl(symbol);
    obl.init(ts1);
    while(obl.t < ts1) {
        obl.next();
    }

    size_t count = 0;
    size_t count_levels = l2 - l1 + 1;
    size_t last_ts = 0;

    ofstream file(config.get_path("data_path") + "files/" + file_name, ios::binary);
    // file structure -> count(size_t), l1(size_t), l2(size_t), records: ts(size_t), count_levels * bids(double), count_levels * asks(double)
    file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(&l1), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(&l2), sizeof(size_t));

    while(!obl.ended() && obl.t <= ts2) {
        if (obl.t - last_ts >= interval) {
            file.write(reinterpret_cast<const char*>(&obl.t), sizeof(size_t));
            for (size_t i = l1; i <= l2; i++) {
                file.write(reinterpret_cast<const char*>(&obl.bids[i]), sizeof(double));
            }
            for (size_t i = l1; i <= l2; i++) {
                file.write(reinterpret_cast<const char*>(&obl.asks[i]), sizeof(double));
            }
            count++;
            last_ts = obl.t;
        }
        obl.next();
    }
    file.seekp(0, ios::beg);
    file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
    file.close();

}

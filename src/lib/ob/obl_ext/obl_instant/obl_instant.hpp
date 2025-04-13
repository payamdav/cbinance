#pragma once
#include "../../obl/obl.hpp"
#include <string>
#include <vector>

using namespace std;



void obl_cache_to_file(string symbol, size_t ts1, size_t ts2, string file_name, size_t level_count);

struct OBL_INSTANT_REC {
    size_t t;
    vector<double> bids;
    vector<double> asks;
    size_t bids_start_level;
    size_t asks_start_level;
};

struct OBL_INSTANT_SCORE_REC {
    size_t t;
    vector<int> bids;
    vector<int> asks;
    int bids_start_level;
    int asks_start_level;
};

struct OBL_INSTANT_AVG_REC {
    size_t t;
    double b;
    double a;
};


vector<OBL_INSTANT_REC> obl_cache(string symbol, size_t ts1, size_t ts2, size_t level_count);
vector<OBL_INSTANT_SCORE_REC> obl_cache_score(vector<OBL_INSTANT_REC> & records);

vector<OBL_INSTANT_REC> obl_cache_ratio_to_vols(string symbol, size_t ts1, size_t ts2, size_t level_count, size_t vols_n);
vector<OBL_INSTANT_AVG_REC> obl_cache_ratio_to_vols_avg(string symbol, size_t ts1, size_t ts2, size_t level_count, size_t vols_n, string file_name="");
void obl_limited_snapshots_to_file(string symbol, size_t ts1, size_t ts2, size_t l1, size_t l2, size_t interval, string file_name);

template<typename T>
vector<vector<double>> obl_cache_flatten(vector<T> & records) {
    size_t count = records.size();
    size_t level_count = records[0].bids.size();
    vector<vector<double>> flattened(2, vector<double>(level_count * count));
    for (size_t i = 0; i < count; i++) {
        for (size_t j = 0; j < level_count; j++) {
            flattened[0][i * level_count + j] = records[i].bids[j];
            flattened[1][i * level_count + j] = records[i].asks[j];
        }
    }
    return flattened;
}

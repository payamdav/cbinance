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


vector<OBL_INSTANT_REC> obl_cache(string symbol, size_t ts1, size_t ts2, size_t level_count);
vector<OBL_INSTANT_SCORE_REC> obl_cache_score(vector<OBL_INSTANT_REC> & records);

vector<OBL_INSTANT_REC> obl_cache_ratio_to_vols(string symbol, size_t ts1, size_t ts2, size_t level_count, size_t vols_n);

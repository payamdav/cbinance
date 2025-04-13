#include <iostream>
#include <string>
#include "../../lib/utils/timer.hpp"
#include "../../lib/utils/datetime_utils.hpp"
#include "../../lib/ob/obl_ext/obl_instant/obl_instant.hpp"
#include "../../lib/statistics/basic_statistics.hpp"
#include <ranges>


using namespace std;

void obl_cache_to_file_test() {
    string symbol = "btcusdt";
    size_t ts1 = utils::get_timestamp("2025-03-18 00:00:00");
    size_t ts2 = utils::get_timestamp("2025-03-18 01:00:00");
    string file_name = "test_obl_cache.bin";
    size_t level_count = 10;

    obl_cache_to_file(symbol, ts1, ts2, file_name, level_count);
}

void obl_cache_test(string symbol, size_t ts1, size_t ts2, size_t level_count) {
    // vector<OBL_INSTANT_REC> records = obl_cache(symbol, ts1, ts2, level_count);
    // cout << "Records count: " << records.size() << endl;
    // vector<OBL_INSTANT_SCORE_REC> score = obl_cache_score(records);
    // cout << "Score count: " << score.size() << endl;
    // cout << "-------------------------" << endl;
    vector<OBL_INSTANT_REC> records_v = obl_cache_ratio_to_vols(symbol, ts1, ts2, level_count, 10);
    cout << "Records count: " << records_v.size() << endl;
    vector<OBL_INSTANT_SCORE_REC> score_v = obl_cache_score(records_v);
    cout << "Score count: " << score_v.size() << endl;
    cout << "-------------------------" << endl;

}

void obl_cache_histogram_test(string symbol, size_t ts1, size_t ts2, size_t level_count) {
    vector<OBL_INSTANT_REC> records = obl_cache(symbol, ts1, ts2, level_count);
    cout << "Records count: " << records.size() << endl;
    auto flattened = obl_cache_flatten(records);
    cout << "Flattened count: " << flattened[0].size() << " " << flattened[1].size() << endl;
    cout << "-------------------------" << endl;
    histogram(flattened[0], 100, "bids_histogram.csv");
    histogram(flattened[1], 100, "asks_histogram.csv");
    cout << "-------------------------" << endl;
}

void obl_cache_vols_histogram_test(string symbol, size_t ts1, size_t ts2, size_t level_count) {
    vector<OBL_INSTANT_REC> records = obl_cache_ratio_to_vols(symbol, ts1, ts2, level_count, 10);
    cout << "Records count: " << records.size() << endl;
    auto flattened = obl_cache_flatten(records);
    cout << "Flattened count: " << flattened[0].size() << " " << flattened[1].size() << endl;
    cout << "-------------------------" << endl;
    histogram(flattened[0], 100, "bids_vols_histogram.csv");
    histogram(flattened[1], 100, "asks_vols_histogram.csv");
    cout << "-------------------------" << endl;
}

void obl_cache_vols_avg_histogram_test(string symbol, size_t ts1, size_t ts2, size_t level_count, size_t vols_n) {
    vector<OBL_INSTANT_AVG_REC> records = obl_cache_ratio_to_vols_avg(symbol, ts1, ts2, level_count, vols_n, "obl_cache_vols_avg.bin");
    cout << "Records count: " << records.size() << endl;
    histogram(records | views::transform([](auto & rec) { return rec.b; }) | ranges::to<vector<double>>(), 100, "bids_vols_avg_histogram.csv");
    histogram(records | views::transform([](auto & rec) { return rec.a; }) | ranges::to<vector<double>>(), 100, "asks_vols_avg_histogram.csv");
    cout << "-------------------------" << endl;
}

int main() {
    utils::Timer timer;
    string symbol = "vineusdt";
    // obl_cache_histogram_test(symbol, utils::get_timestamp("2025-03-18 00:00:00"), utils::get_timestamp("2025-03-21 01:00:00"), 10);
    // obl_cache_vols_histogram_test(symbol, utils::get_timestamp("2025-03-18 00:00:00"), utils::get_timestamp("2025-03-21 01:00:00"), 10);
    obl_cache_vols_avg_histogram_test(symbol, utils::get_timestamp("2025-03-18 00:00:00"), utils::get_timestamp("2025-03-21 01:00:00"), 10, 10);

    timer.checkpoint("obl_cache_test");
    return 0;
}

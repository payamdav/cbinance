#include <iostream>
#include <string>
#include "../../lib/utils/timer.hpp"
#include "../../lib/utils/datetime_utils.hpp"
#include "../../lib/ob/obl_ext/obl_instant/obl_instant.hpp"


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

int main() {
    utils::Timer timer;
    obl_cache_test("btcusdt", utils::get_timestamp("2025-03-18 00:00:00"), utils::get_timestamp("2025-03-21 01:00:00"), 10);

    timer.checkpoint("obl_cache_test");
    return 0;
}

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
int main() {
    obl_cache_to_file_test();
    return 0;
}
#include <string>
#include "../../../lib/ta/candle_anal/candle_anal.hpp"
#include <iostream>
#include "../../../lib/utils/datetime_utils.hpp"
#include "../../../lib/config/config.hpp"

using namespace std;


void test1(string symbol) {
    size_t ts1 = utils::get_timestamp("2025-03-14 00:00:00");
    size_t ts2 = utils::get_timestamp("2025-03-15 00:00:00");

    CandleAnal candleAnal(symbol, ts1, ts2);

}


int main() {
    auto symbols = config.get_csv_strings("symbols_list");
    for (const auto & symbol : symbols) {
        // cout << "Testing symbol: " << symbol << endl;
        test1(symbol);
    }
    return 0;
}

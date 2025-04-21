#include <iostream>
#include <string>
#include "../../lib/utils/timer.hpp"
#include "../../lib/utils/datetime_utils.hpp"
#include "../../lib/binance_candles/binance_candles.hpp"
#include "../../lib/ta/pip_levelizer/pip_levelizer.hpp"
#include "../../lib/ta/vols/vols.hpp"
#include "../../lib/config/config.hpp"


using namespace std;


void candles_test() {
    string symbol = "btcusdt";
    size_t ts1 = utils::get_timestamp("2025-03-18 01:00:00");
    size_t ts2 = utils::get_timestamp("2025-03-19 01:00:00");

    PipLevelizer levelizer(symbol);

    BinanceCandles candles(symbol, ts1, ts2, &levelizer);
    cout << candles << endl;
    cout << "is_sorted: " << candles.is_sorted() << endl;
    cout << "is_any_gap: " << candles.is_any_gap() << endl;
    cout << "First candle: " << candles[0] << endl;
    cout << "Last candle: " << candles[candles.size() - 1] << endl;

    Vols vols(candles, 5);
    cout << "Vols: " << vols.ts.size() << " - v: " << vols.v.size() << endl;
    cout << format("ts: {} - v: {} - b: {} - s: {}", vols.ts[0], vols.v[0], vols.b[0], vols.s[0]) << endl;
    cout << format("ts: {} - v: {} - b: {} - s: {}", vols.ts[1], vols.v[1], vols.b[1], vols.s[1]) << endl;


}

void candles_volume_report(string symbol) {
    size_t ts1 = utils::get_timestamp("2025-03-10 00:00:00");
    size_t ts2 = utils::get_timestamp("2025-03-30 23:59:59");

    PipLevelizer levelizer(symbol);

    BinanceCandles candles(symbol, ts1, ts2, &levelizer);
    cout << candles << endl;

    double volume_sum = 0;
    double volume_buy_sum = 0;
    double volume_sell_sum = 0;
    size_t count = 0;

    for (BinanceCandle &candle : candles) {
        volume_sum += candle.v;
        volume_buy_sum += candle.vb;
        volume_sell_sum += candle.vs;
        count++;
    }

    volume_sum /= count;
    volume_buy_sum /= count;
    volume_sell_sum /= count;

    cout << "Average Volume: " << volume_sum << endl;
    cout << "Average Buy Volume: " << volume_buy_sum << endl;
    cout << "Average Sell Volume: " << volume_sell_sum << endl;
    cout << "Total Candles Processed: " << count << endl;
}


int main() {
    for (string symbol : config.get_csv_strings("symbols_list")) {
        candles_volume_report(symbol);
        cout << "----------------------------------------" << endl;
    }
    return 0;
}

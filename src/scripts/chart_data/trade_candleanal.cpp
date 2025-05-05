#include <iostream>
#include <string>
#include <fstream>
#include <ranges>
#include "../../lib/config/config.hpp"
#include "../../lib/utils/datetime_utils.hpp"
#include "../../lib/utils/string_utils.hpp"
#include "../../lib/trade/tl.hpp"
#include "../../lib/ta/zigzag/zigzag.hpp"
#include "../../lib/ta/candle_anal/candle_anal.hpp"

using namespace std;



void create_chart_data_files_for_trade_candleanal(string symbol, string dt1, string dt2) {
    symbol = utils::toLowerCase(symbol); // Ensure the symbol
    size_t ts1 = utils::get_timestamp(dt1);
    size_t ts2 = utils::get_timestamp(dt2);
    size_t l1 = 0;
    size_t l2 = 0;

    // writing Trades (tls)
    TLS tls(symbol);
    tls.open()->read_by_ts(ts1, ts2)->save_lite("trades_chart_data.bin")->min_max_level(l1, l2); // Read trades from the file and save them to a binary file

    // writing CandleAnal
    CandleAnal candleAnal(symbol, ts1, ts2); // Create a CandleAnal object
    candleAnal.save_vwl("/home/payam/data/files/candles_vwl.bin"); // Save the VWL data to a file
    int avg_candle_size = candleAnal.average_candle_size(); // Calculate the average candle size

    cout << "Average candle size: " << avg_candle_size << endl; // Print the average candle size

    auto va = candleAnal.volume_areas(121, 100); // Get the volume areas
    cout << "Volume areas: " << endl; // Print the volume areas
    for (const auto & area : va) {
        cout << "Area: " << area.ts_start << " " << area.ts_end << " " << area.level_center << " " << area.avg_volume << endl; // Print each volume area
    }
    cout << "Volume areas size: " << va.size() << endl; // Print the size of the volume areas
    CandleAnal::save_volume_areas(va, "/home/payam/data/files/volume_areas.bin"); // Save the volume areas to a binary file


    ZigZag_TL zz1(avg_candle_size);
    ZigZag_TL zz2(avg_candle_size * 2);
    ZigZag_TL zz3(avg_candle_size * 3);

    zz1.push(tls)->save("zigzag_1.bin"); // Create a ZigZag_TL and save it to a file
    zz2.push(tls)->save("zigzag_2.bin"); // Create a ZigZag_TL and save it to a file
    zz3.push(tls)->save("zigzag_3.bin"); // Create a ZigZag_TL and save it to a file

    cout << "Zigzag 1: " << zz1.stat() << endl; // Print the statistics of the first ZigZag_TL
    cout << "Zigzag 2: " << zz2.stat() << endl; // Print the statistics of the second ZigZag_TL
    cout << "Zigzag 3: " << zz3.stat() << endl; // Print the statistics of the third ZigZag_TL

}
 

int main(int argc, char *argv[]) {
    if (argc == 4) {
        string symbol = argv[1]; // Get the symbol from command line argument
        string dt1 = string(argv[2]).replace(10, 1, " "); // Get the start datetime from command line argument
        string dt2 = string(argv[3]).replace(10, 1, " "); // Get the end datetime from command line argument
        create_chart_data_files_for_trade_candleanal(symbol, dt1, dt2); // Call the function to create chart data files
    }
    else {
        cerr << "Usage: " << argv[0] << " <symbol> <datetime1> <datetime2>" << " datetime format: YYYY-MM-DD:HH:MM:SS" << endl; // Print usage information
        return 1; // Exit with error if not enough arguments
    }
    return 0; // Exit successfully
}
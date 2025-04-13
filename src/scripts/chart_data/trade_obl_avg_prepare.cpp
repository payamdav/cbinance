#include <iostream>
#include <string>
#include <fstream>
#include <ranges>
#include "../../lib/config/config.hpp"
#include "../../lib/utils/datetime_utils.hpp"
#include "../../lib/utils/string_utils.hpp"
#include "../../lib/trade/tl.hpp"
#include "../../lib/ta/zigzag/zigzag.hpp"
#include "../../lib/ob/obl_ext/obl_instant/obl_instant.hpp"

using namespace std;



void create_chart_data_files_for_trade_obl_instant_vols_avg(string symbol, string dt1, string dt2, size_t level_count, size_t vols_n) {
    symbol = utils::toLowerCase(symbol); // Ensure the symbol
    size_t ts1 = utils::get_timestamp(dt1);
    size_t ts2 = utils::get_timestamp(dt2);

    // writing Trades (tls)
    TLS tls(symbol);
    tls.open()->read_by_ts(ts1, ts2)->save_lite("trades_chart_data.bin"); // Read trades from the file and save them to a binary file
    (new ZigZag_TL(10))->push(tls)->save("zigzag_10.bin"); // Create a ZigZag_TL and save it to a file
    (new ZigZag_TL(30))->push(tls)->save("zigzag_30.bin"); // Create a ZigZag_TL and save it to a file
    (new ZigZag_TL(200))->push(tls)->save("zigzag_200.bin"); // Create a ZigZag_TL and save it to a file
    // writing OBL
    obl_cache_ratio_to_vols_avg(symbol, ts1, ts2, level_count, vols_n, "obl_cache_vols_avg.bin");

}


int main(int argc, char *argv[]) {
    if (argc != 6) {
        cerr << "Usage: " << argv[0] << " <symbol> <datetime1> <datetime2> <level_count> <vols_n>" << " datetime format: YYYY-MM-DD:HH:MM:SS" << endl; // Print usage information
        return 1; // Exit with error if not enough arguments
    }
    string symbol = argv[1]; // Get the symbol from command line argument
    string dt1 = string(argv[2]).replace(10, 1, " "); // Get the start datetime from command line argument
    string dt2 = string(argv[3]).replace(10, 1, " "); // Get the end datetime from command line argument
    size_t level_count = atoi(argv[4]); // Get the level count from command line argument
    size_t vols_n = atoi(argv[5]); // Get the number of volumes from command line argument
    create_chart_data_files_for_trade_obl_instant_vols_avg(symbol, dt1, dt2, level_count, vols_n); // Call the function to create chart data files
    return 0; // Exit successfully
}
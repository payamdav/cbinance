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



void create_chart_data_files_for_trade_obl_instant_snapshot(string symbol, string dt1, string dt2, size_t interval) {
    symbol = utils::toLowerCase(symbol); // Ensure the symbol
    size_t ts1 = utils::get_timestamp(dt1);
    size_t ts2 = utils::get_timestamp(dt2);
    size_t l1 = 0;
    size_t l2 = 0;

    // writing Trades (tls)
    TLS tls(symbol);
    tls.open()->read_by_ts(ts1, ts2)->save_lite("trades_chart_data.bin")->min_max_level(l1, l2); // Read trades from the file and save them to a binary file
    (new ZigZag_TL(10))->push(tls)->save("zigzag_10.bin"); // Create a ZigZag_TL and save it to a file
    (new ZigZag_TL(30))->push(tls)->save("zigzag_30.bin"); // Create a ZigZag_TL and save it to a file
    (new ZigZag_TL(200))->push(tls)->save("zigzag_200.bin"); // Create a ZigZag_TL and save it to a file
    // writing OBL
    obl_limited_snapshots_to_file(symbol, ts1, ts2, l1, l2, interval, "obl_cache_snapshot.bin"); // Create a file with OBL snapshots

}
 
void create_chart_data_files_for_trade_obl_instant_snapshot_full(string symbol, string dt1, string dt2, size_t interval) {
    symbol = utils::toLowerCase(symbol); // Ensure the symbol
    size_t ts1 = utils::get_timestamp(dt1);
    size_t ts2 = utils::get_timestamp(dt2);
    size_t l1 = 0;
    size_t l2 = 0;

    // writing Trades (tls)
    TLS tls(symbol);
    tls.open()->read_by_ts(ts1, ts2)->save("trades_chart_data.bin")->min_max_level(l1, l2); // Read trades from the file and save them to a binary file
    (new ZigZag_TL(10))->push(tls)->save("zigzag_10.bin"); // Create a ZigZag_TL and save it to a file
    (new ZigZag_TL(30))->push(tls)->save("zigzag_30.bin"); // Create a ZigZag_TL and save it to a file
    (new ZigZag_TL(200))->push(tls)->save("zigzag_200.bin"); // Create a ZigZag_TL and save it to a file
    // writing OBL
    obl_limited_snapshots_to_file(symbol, ts1, ts2, l1, l2, interval, "obl_cache_snapshot.bin"); // Create a file with OBL snapshots

}

int main(int argc, char *argv[]) {
    if (argc == 5) {
        string symbol = argv[1]; // Get the symbol from command line argument
        string dt1 = string(argv[2]).replace(10, 1, " "); // Get the start datetime from command line argument
        string dt2 = string(argv[3]).replace(10, 1, " "); // Get the end datetime from command line argument
        size_t interval = atoi(argv[4]); // Get the interval from command line argument
        create_chart_data_files_for_trade_obl_instant_snapshot(symbol, dt1, dt2, interval); // Call the function to create chart data files
        // create_chart_data_files_for_trade_obl_instant_snapshot_full(symbol, dt1, dt2, interval); // Call the function to create chart data files
    }
    else if (argc == 6) {
        string symbol = argv[1]; // Get the symbol from command line argument
        string dt1 = string(argv[2]).replace(10, 1, " "); // Get the start datetime from command line argument
        string dt2 = string(argv[3]).replace(10, 1, " "); // Get the end datetime from command line argument
        size_t interval = atoi(argv[4]); // Get the interval from command line argument
        string cmd = argv[5]; // Get the command from command line argument
        if (cmd == "full") {
            create_chart_data_files_for_trade_obl_instant_snapshot_full(symbol, dt1, dt2, interval); // Call the function to create chart data files
        }
        else if (cmd == "lite") {
            create_chart_data_files_for_trade_obl_instant_snapshot(symbol, dt1, dt2, interval); // Call the function to create chart data files
        }
        else {
            cerr << "Unknown command: " << cmd << endl; // Print error message for unknown command
            return 1; // Exit with error
        }
    }
    else {
        cerr << "Usage: " << argv[0] << " <symbol> <datetime1> <datetime2> <interval> [full | lite]" << " datetime format: YYYY-MM-DD:HH:MM:SS" << endl; // Print usage information
        return 1; // Exit with error if not enough arguments
    }
    return 0; // Exit successfully
}
#include "../../lib/trade/tl.hpp"
#include "../../lib/config/config.hpp"
#include "../../lib/utils/datetime_utils.hpp"
#include "../../lib/utils/string_utils.hpp"
#include <iostream>
#include "../../lib/ta/pip_levelizer/pip_levelizer.hpp"
#include "../../lib/utils/timer.hpp"
#include <string>
#include <fstream>

using namespace std;



void create_chart_data_files_for_trade(string symbol, string start_datetime, int minutes) {
    size_t start_ts = utils::get_timestamp(start_datetime);
    size_t end_ts = start_ts + minutes * 60 * 1000;
    symbol = utils::toLowerCase(symbol); // Ensure the symbol
    TLS tls(symbol);
    tls.open(); // Open the TLS for reading
    tls.read_by_ts(start_ts, end_ts); // Read TLs within the specified timestamp range

    ofstream trades_data_file(config.get_path("data_path") + "files/trades_chart_data_" + symbol + ".bin", ios::binary); // Open a binary file to save the chart data
    for (size_t i = 0; i < tls.size(); i++) { // Iterate over each TL in the TLS
        trades_data_file.write(reinterpret_cast<const char*>(&tls[i].t), sizeof(size_t)); // Write the start timestamp of the TL
        trades_data_file.write(reinterpret_cast<const char*>(&tls[i].l), sizeof(size_t)); // Write the level of the TL
    }
    trades_data_file.close(); // Close the file after writing all TLs
    tls.close(); // Close the TLS after reading
}


int main(int argc, char *argv[]) {
    if (argc < 5) {
        cerr << "Usage: " << argv[0] << " <symbol> <start_datetime> <minutes>" << " datetime format: YYYY-MM-DD HH:MM:SS" << endl; // Print usage information
        return 1; // Exit with error if not enough arguments
    }
    string symbol = argv[1]; // Get the symbol from command line argument
    string start_datetime = string(argv[2]) + " " + string(argv[3]); // Get the start datetime from command line arguments
    int minutes = atoi(argv[4]); // Get the number of minutes from command line argument
    create_chart_data_files_for_trade(symbol, start_datetime, minutes); // Call the function to create chart data files
}
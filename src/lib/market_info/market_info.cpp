#include "market_info.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include "../utils/string_utils.hpp"
#include "../config/config.hpp"


using namespace std;

MarketInfo::MarketInfo() {
    base_path = config.get_path("data_path") + "um/market_info/";
    symbols_list = config.get_csv_strings("symbols_list");
}


void MarketInfo::set_min_max_price(const string &symbol, double min_price, double max_price) {
    // Set the min and max prices for a given symbol
    string file_path = base_path + utils::toLowerCase(symbol) + ".bin";
    ofstream file(file_path, ios::binary | ios::out);
    if (!file) {
        cerr << "Error opening file for writing: " << file_path << endl;
        return;
    }
    // Write the min and max prices to the binary file
    file.write(reinterpret_cast<const char*>(&min_price), sizeof(min_price));
    file.write(reinterpret_cast<const char*>(&max_price), sizeof(max_price));
    if (!file) {
        cerr << "Error writing to file: " << file_path << endl;
    } else {
        cout << "Successfully set min/max prices for symbol: " << symbol << " to min: " << min_price << ", max: " << max_price << endl;
    }
    file.close(); // Close the file after writing
}


void MarketInfo::get_min_max_price(const string &symbol, double &min_price, double &max_price) {
    // Get the min and max prices for a given symbol
    string file_path = base_path + utils::toLowerCase(symbol) + ".bin";
    ifstream file(file_path, ios::binary | ios::in);
    if (!file) {
        cerr << "Error opening file for reading: " << file_path << endl;
        return;
    }
    // Read the min and max prices from the binary file
    file.read(reinterpret_cast<char*>(&min_price), sizeof(min_price));
    file.read(reinterpret_cast<char*>(&max_price), sizeof(max_price));
    if (!file) {
        cerr << "Error reading from file: " << file_path << endl;
    } else {
        // cout << "Successfully retrieved min/max prices for symbol: " << symbol << " - min: " << min_price << ", max: " << max_price << endl;
    }
    file.close(); // Close the file after reading
}

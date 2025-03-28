#include "ob_min_max_price.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include "../../config/config.hpp"
#include "../../utils/string_utils.hpp"


using namespace std;

OBMinMaxPrice::OBMinMaxPrice(string symbol) : OB(symbol) {
    min_price = 1e9; // Set initial min price to a very high value
    max_price = 0;   // Set initial max price to a very low value
}

void OBMinMaxPrice::on_after_update() {
    double best_bid = bids.rbegin()->first; // Get the best bid price
    double best_ask = asks.begin()->first; // Get the best ask price
    if (best_bid < min_price) {
        min_price = best_bid; // Update min price if best bid is lower
    }
    if (best_ask > max_price) {
        max_price = best_ask; // Update max price if best ask is higher
    }
}

void OBMinMaxPrice::save_min_max_price() {
    string file_path = config.get_path("data_path") + "um/depth/" + utils::toLowerCase(symbol) + "/" + "symbol_info.bin";
    ofstream file(file_path, ios::binary | ios::trunc);
    if (!file) {
        cerr << "Error opening file for writing: " << file_path << endl;
        return;
    }
    file.write(reinterpret_cast<const char*>(&min_price), sizeof(min_price));
    file.write(reinterpret_cast<const char*>(&max_price), sizeof(max_price));
    if (!file) {
        cerr << "Error writing to file: " << file_path << endl;
    } else {
        // cout << "Min and Max prices saved successfully for symbol: " << symbol << endl;
    }
    // Close the file
    file.close();
    // Optionally, you can print the min and max prices to the console for verification
    // cout << "Current Min Price: " << min_price << endl;
    // cout << "Current Max Price: " << max_price << endl;
}

bool read_min_max_price(string symbol, double &min_price, double &max_price) {
    string file_path = config.get_path("data_path") + "um/depth/" + utils::toLowerCase(symbol) + "/" + "symbol_info.bin";
    ifstream file(file_path, ios::binary);
    if (!file) {
        cerr << "Error opening file for reading: " << file_path << endl;
        return false; // Return false if the file cannot be opened
    }
    file.read(reinterpret_cast<char*>(&min_price), sizeof(min_price));
    file.read(reinterpret_cast<char*>(&max_price), sizeof(max_price));
    if (!file) {
        cerr << "Error reading from file: " << file_path << endl;
        return false; // Return false if there was an error reading the data
    }
    // Close the file
    file.close();
    return true; // Return true if min and max prices were read successfully
}
#include "../../lib/trade/trade.hpp"
#include "../../lib/config/config.hpp"
#include "../../lib/market_info/market_info.hpp"
#include <iostream>
#include <vector>
#include <string>


using namespace std;

int main(int argc, char *argv[]) {
    vector<string> symbols = config.get_csv_strings("symbols_list");
    cout << "Usage: " << endl << argv[0] << " - set min/max price for all symbols from binary files" << endl << argv[0] << " show  - show min/max price for all symbols from binary files" << endl;
    MarketInfo market_info; // Create a MarketInfo object to get the symbols list

    if (argc == 1) {
        for (const auto& symbol : symbols) {
            double min_price, max_price;
            Trades trades(symbol); // Create a Trades object for the symbol
            trades.min_max_price_from_file_iteration_in_chunks(min_price, max_price); // Calculate min and max prices from the binary file
            // Print the min and max prices for the symbol
            trades.close(); // Close the binary file after processing
            cout << "Symbol: " << symbol << " - Min Price: " << min_price << ", Max Price: " << max_price << endl; // Print the min and max prices
            market_info.set_min_max_price(symbol, min_price, max_price); // Set the min and max prices in the market info
        }
    }
    else {
        for (const auto& symbol : symbols) {
            double min_price, max_price;
            market_info.get_min_max_price(symbol, min_price, max_price); // Get the min and max prices from the market info
            cout << "Symbol: " << symbol << " - Min Price: " << min_price << ", Max Price: " << max_price << endl; // Print the min and max prices
        }
    }
    return 0;
}

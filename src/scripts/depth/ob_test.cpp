#include <iostream>
#include <string>
// #include <vector>
// #include "../../lib/config/config.hpp"
// #include "../../lib/ob/snapshot.hpp"
// #include "../../lib/ob/update.hpp"
#include "../../lib/ob/ob.hpp"
#include "../../lib/ob/obl_creator/obl_creator.hpp"
#include "../../lib/ob/obtools/ob_min_max_price.hpp" // Include the OBMinMaxPrice header for the min/max price functionality
#include "../../lib/utils/timer.hpp" // Include the timer utility for performance measurement

using namespace std;



void ob_test_1(int argc, char *argv[]) {
    // auto symbols = config.get_csv_strings("symbols_list");
    // OB ob("btcusdt");
    OB ob("adausdt");
    // ob.build(1742319330205);
    ob.build();
    ob.check(); // This will check the order book for any inconsistencies or issues
}

class MyOB : public OB {
    public:
        MyOB(string symbol) : OB(symbol) {}
        size_t count_negative_spreads = 0; // Count of negative spreads
        double max_spread = 0; // Maximum spread observed
        double min_spread = 1e9; // Minimum spread observed
        double max_spread_percent = 0; // Maximum spread percentage observed

        void on_after_update() override {
            double best_bid = bids.rbegin()->first; // Get the best bid price
            double best_ask = asks.begin()->first; // Get the best ask price
            double spread = best_ask - best_bid; // Calculate the spread
            if (spread < 0) {
                count_negative_spreads++; // Increment count if spread is negative
            }
            else {
                // Update max and min spreads
                if (spread > max_spread) {
                    max_spread = spread; // Update maximum spread
                }
                if (spread < min_spread) {
                    min_spread = spread; // Update minimum spread
                }
                // Calculate spread percentage
                double spread_percent = (spread / best_bid) * 100;
                if (spread_percent > max_spread_percent) {
                    max_spread_percent = spread_percent; // Update maximum spread percentage
                }
            }
        }

        void print_spread_statistics() {
            cout << "Spread Statistics for " << symbol << ":" << endl;
            cout << "Count of Negative Spreads: " << count_negative_spreads << endl;
            cout << "Maximum Spread: " << max_spread << endl;
            cout << "Minimum Spread: " << min_spread << endl;
            cout << "Maximum Spread Percentage: " << max_spread_percent << "%" << endl;
        }
};

void ob_test_2(int argc, char *argv[]) {
    // auto symbols = config.get_csv_strings("symbols_list");
    // MyOB ob("btcusdt");
    MyOB ob("trumpusdt");
    // MyOB ob("adausdt");
    // ob.build(1742319330205);
    ob.build();
    ob.check(); // This will check the order book for any inconsistencies or issues
    // After building the order book, print the spread statistics
    ob.print_spread_statistics(); // Print the spread statistics
}

void test_min_max_price(int argc, char *argv[]) {
    auto symbols = config.get_csv_strings("symbols_list");
    for (const auto& symbol : symbols) {
        // Create an instance of OBMinMaxPrice for each symbol
        OBMinMaxPrice ob(symbol);
        // Build the order book for the symbol
        ob.build();
        // Check for any inconsistencies or issues in the order book
        ob.check();
        // Save the min and max prices to a binary file
        ob.save_min_max_price();
    }
}

void test_min_max_price_read(int argc, char *argv[]) {
    auto symbols = config.get_csv_strings("symbols_list");
    double min_price, max_price; // Variables to hold min and max prices
    for (const auto& symbol : symbols) {
        bool res = read_min_max_price(symbol, min_price, max_price); // Read the min and max prices from the binary file
        if (res) {
            // If reading was successful, print the min and max prices
            cout << "Symbol: " << symbol << ", Min Price: " << min_price << ", Max Price: " << max_price << endl;
        } else {
            // If there was an error reading the prices, print an error message
            cout << "Failed to read min/max prices for symbol: " << symbol << endl;
        }
    }
}

void test_obl_creator(string symbol) {
    utils::Timer timer(symbol + "_obl_creator_timer"); // Timer for performance measuremen
    OBLC obl(symbol);
    obl.build();
    obl.update_idx.close(); // Close the index file after building
    obl.update_data.close(); // Close the binary file after building
    timer.checkpoint(); // Checkpoint after building the order book
}


int main(int argc, char *argv[]) {
    test_obl_creator("btcusdt"); // Test the OBLC creator for a specific symbol

    return 0;
}

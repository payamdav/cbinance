#include "../../lib/trade/trade.hpp"
#include "../../lib/config/config.hpp"
#include <iostream>
#include "../../lib/ta/pip_levelizer/pip_levelizer.hpp"
#include "../../lib/utils/timer.hpp"


using namespace std;


void test_trade_1() {
    string symbol = "adausdt";
    Trades trades(symbol);
    cout << "Total trades in binary file: " << trades.size() << " - count: " << trades.count << endl;
    trades.open(); // Open the binary file for reading
    cout << "First Trade: " << trades.read_first() << endl; // Read the first trade
    cout << "Last Trade: " << trades.read_last() << endl; // Read the last trade
    size_t time_to_search = 1741864804606; // Example timestamp to search for
    // size_t time_to_search = 1742947199717; // Example timestamp to search for
    size_t index = trades.search(time_to_search); // Search for a trade with timestamp >= 1741564804605
    Trade trade = trades.read_trade(index); // Read the trade at the found index
    cout << "Trade found at index " << index << " for ts: " << time_to_search << ": " << trade << endl; // Print the found trade
    trades.read_by_index(1000, 10000);
    cout << "trade size: " << trades.size() << endl;
    cout << "Last Trade: " << *trades.rbegin() << endl; // Print the last trade after reading by index
    trades.read_by_ts(1741864804600, 1741864805700); // Read trades by timestamp range
    cout << "trade size: " << trades.size() << endl;
    cout << "Last Trade: " << *trades.rbegin() << endl; // Print the last trade after reading by index
    trades.close(); // Close the binary file
}

void min_max_price(string symbol, double &min_price, double &max_price) {
    Trades trades(symbol);
    // cout << "Total trades in binary file: " << trades.size() << " - count: " << trades.count << endl;
    trades.open(); // Open the binary file for reading
    size_t readed = 0;
    size_t chunk = 100000;
    min_price = 1e10; // Initialize min_price to a high value
    max_price = 0.0; // Initialize max_price to a low value
    while(readed < trades.count) {
        // Read a chunk of trades
        trades.read_by_index(readed, chunk);
        size_t chunk_size = trades.size(); // Get the size of trades after reading by index
        // cout << "Read " << chunk_size << " trades from index " << readed << " to " << (readed + chunk_size - 1) << endl; // Print the range of trades read
        for (Trade &trade : trades) { // Iterate over the trades in the vector
            if (trade.p < min_price) {
                min_price = trade.p; // Update min_price if current trade price is lower
            }
            if (trade.p > max_price) {
                max_price = trade.p; // Update max_price if current trade price is higher
            }
        }

        readed += chunk_size; // Update the number of trades read


    }

    trades.close(); // Close the binary file

}

void pip_levelizer_test(string symbol, PipLevelizer &pip_levelizer) {
    utils::Timer timer(symbol);
    Trades trades(symbol);
    trades.open(); // Open the binary file for reading
    size_t readed = 0;
    size_t chunk = 100000;
    size_t l1, l2, count_error = 0;
    while(readed < trades.count) {
        trades.read_by_index(readed, chunk);
        size_t chunk_size = trades.size(); // Get the size of trades after reading by index
        for (Trade &trade : trades) { // Iterate over the trades in the vector
            l1 = pip_levelizer.get_level(trade.p); // Get the level for the trade price using the PipLevelizer
            l2 = pip_levelizer.get_level_binary_search(trade.p); // Get the level using binary search method
            if (l1 != l2) {
                count_error++; // Increment the error count if the levels do not match
                // cout << "Error: Levels do not match for trade price: " << trade.p << ", Level1: " << l1 << ", Level2: " << l2 << endl; // Print the error message
            }
        }

        readed += chunk_size; // Update the number of trades read
        // timer.checkpoint(to_string(readed) + " trades processed");


    }
    timer.checkpoint("Finalizing"); // Final checkpoint for the timer

    trades.close(); // Close the binary file

    // Print the total number of errors found
    if (count_error > 0) {
        cout << "Total errors found: " << count_error << endl; // Print the total number of errors
    } else {
        cout << "No errors found in level calculation for symbol: " << symbol << endl; // Print success message
    }

}

void pip_levelizer_speedtest(string symbol, PipLevelizer &pip_levelizer) {
    utils::Timer timer(symbol);
    Trades trades(symbol);
    trades.open(); // Open the binary file for reading
    size_t chunk = 100000;
    size_t l1, l2, count_error = 0;
    size_t readed = 0;
    timer.checkpoint("Start for " + symbol);
    while(readed < trades.count) {
        trades.read_by_index(readed, chunk);
        size_t chunk_size = trades.size(); // Get the size of trades after reading by index
        for (Trade &trade : trades) { // Iterate over the trades in the vector
            l1 = pip_levelizer.get_level(trade.p); // Get the level for the trade price using the PipLevelizer
        }
        readed += chunk_size; // Update the number of trades read
    }
    timer.checkpoint("Finished get_level for all trades"); // Checkpoint after processing all trades

    readed = 0;
    while(readed < trades.count) {
        trades.read_by_index(readed, chunk);
        size_t chunk_size = trades.size(); // Get the size of trades after reading by index
        for (Trade &trade : trades) { // Iterate over the trades in the vector
            l1 = pip_levelizer.get_level_binary_search(trade.p); // Get the level using binary search method
        }
        readed += chunk_size; // Update the number of trades read
    }
    timer.checkpoint("Finished get_level_binary_search for all trades"); // Checkpoint after processing all trades

    trades.close(); // Close the binary file


}

void levelizer_test_for_symbol(string symbol) {
    double min_price, max_price;
    min_max_price(symbol, min_price, max_price); // Get the min and max prices for the symbol
    // cout << "Min Price: " << min_price << ", Max Price: " << max_price << endl; // Print the min and max prices

    // Create a PipLevelizer instance with the calculated min and max prices
    PipLevelizer pip_levelizer(min_price * 0.98, max_price * 1.02, 0.0001);
    // cout << "Pip Levelizer Levels: " << pip_levelizer << endl; // Print the levels generated by PipLevelizer
    // Test the PipLevelizer with the trades data
    pip_levelizer_test(symbol, pip_levelizer); // Test the PipLevelizer with the trades data for the given symbol

}

void levelizer_speedtest_for_symbol(string symbol) {
    double min_price, max_price;
    min_max_price(symbol, min_price, max_price); // Get the min and max prices for the symbol
    // cout << "Min Price: " << min_price << ", Max Price: " << max_price << endl; // Print the min and max prices

    // Create a PipLevelizer instance with the calculated min and max prices
    PipLevelizer pip_levelizer(min_price * 0.98, max_price * 1.02, 0.0001);
    // cout << "Pip Levelizer Levels: " << pip_levelizer << endl; // Print the levels generated by PipLevelizer
    // Test the PipLevelizer with the trades data
    pip_levelizer_speedtest(symbol, pip_levelizer); // Test the PipLevelizer with the trades data for the given symbol

}

void levelizer_test_for_all_symbols() {
    vector<string> symbols = config.get_csv_strings("symbols_list"); // Get the list of symbols from the configuration
    for (const auto& symbol : symbols) {
        // cout << "Testing levelizer for symbol: " << symbol << endl; // Print the current symbol being tested
        levelizer_test_for_symbol(symbol); // Call the levelizer test for each symbol
    }
}

void levelizer_speedtest_for_all_symbols() {
    vector<string> symbols = config.get_csv_strings("symbols_list"); // Get the list of symbols from the configuration
    for (const auto& symbol : symbols) {
        // cout << "Testing levelizer for symbol: " << symbol << endl; // Print the current symbol being tested
        levelizer_speedtest_for_symbol(symbol); // Call the levelizer test for each symbol
    }
}


int main(int argc, char *argv[]) {
    levelizer_speedtest_for_all_symbols(); // Run the levelizer test for all symbols

    return 0;
}

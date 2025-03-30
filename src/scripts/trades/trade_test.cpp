#include "../../lib/trade/trade.hpp"
#include "../../lib/trade/tl.hpp"
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

void trade_min_max_finder_test() {
    utils::Timer timer;
    vector<string> symbols = config.get_csv_strings("symbols_list"); // Get the list of symbols from the configuration
    timer.checkpoint("Start"); // Start the timer for the operation
    for (const auto& symbol : symbols) {
        double min_price, max_price;
        Trades trades(symbol); // Create a Trades object for the symbol
        trades.min_max_price_from_file_iteration(min_price, max_price); // Calculate min and max prices from the binary file
        // Print the min and max prices for the symbol
        trades.close(); // Close the binary file after processing
        cout << "Symbol: " << symbol << " - Min Price: " << min_price << ", Max Price: " << max_price << endl; // Print the min and max prices
    }
    timer.checkpoint("Completed min/max price calculation for all symbols"); // Checkpoint after processing all symbols
    for (const auto& symbol : symbols) {
        double min_price, max_price;
        Trades trades(symbol); // Create a Trades object for the symbol
        trades.min_max_price_from_file_iteration_in_chunks(min_price, max_price); // Calculate min and max prices from the binary file
        // Print the min and max prices for the symbol
        trades.close(); // Close the binary file after processing
        cout << "Symbol: " << symbol << " - Min Price: " << min_price << ", Max Price: " << max_price << endl; // Print the min and max prices
    }
    timer.checkpoint("Completed min/max price calculation in chunks for all symbols"); // Final checkpoint for the operation



}

void test_trades_to_tls(string symbol) {
    utils::Timer timer(symbol); // Create a timer for the operation
    Trades trades(symbol); // Create a Trades object for the symbol
    TLS tls(symbol); // Create a TLS object for the symbol
    timer.checkpoint("Start"); // Start the timer for the operation
    tls.import_trades(trades); // Import trades into the TLS
    timer.checkpoint("Imported trades to TLS"); // Checkpoint after importing trades
    tls.save(); // Save the TLS to a binary file
    timer.checkpoint("Saved TLS to file"); // Checkpoint after saving the TLS
    cout << "Tls Size: " << tls.size() << endl; // Print the size of the TLS
    cout << "Tls First: " << (tls.size() > 0 ? tls.front() : TL()) << endl; // Print the first TL in the TLS
    cout << "Tls Last: " << (tls.size() > 0 ? tls.back() : TL()) << endl; // Print the last TL in the TLS
}

void test_tls(string symbol) {
    utils::Timer timer(symbol); // Create a timer for the operation
    TLS tls(symbol); // Create a TLS object for the symbol
    tls.open(); // Open the TLS binary file for reading
    TL tl;
    size_t min_l = 10000000; // Initialize min level to a large number
    size_t max_l = 0; // Initialize max level to 0
    double sum_v = 0.0; // Initialize sum of volume to 0
    double sum_b = 0.0; // Initialize sum of aggressive buyers to 0
    double sum_s = 0.0; // Initialize sum of aggressive sellers to 0
    timer.checkpoint("Start"); // Start the timer for the operation
    for (size_t i = 0; i < tls.count; i++) { // Iterate over each TL in the TLS
        tls.next(tl); // Read the next TL from the binary file
        // Update min and max levels
        if (tl.l < min_l) { 
            min_l = tl.l; // Update min level if current level is lower
        }
        if (tl.l > max_l) { 
            max_l = tl.l; // Update max level if current level is higher
        }
        sum_v += tl.v; // Accumulate the total volume
        sum_b += tl.b; // Accumulate the total volume of aggressive buyers
        sum_s += tl.s; // Accumulate the total volume of aggressive sellers
    }
    tls.close(); // Close the TLS file
    timer.checkpoint("Completed reading TLS"); // Checkpoint after reading all TLs
    // Print the results
    cout << "TLS Summary for symbol: " << symbol << endl; // Print the summary for the symbol
    cout << "Min Level: " << min_l << endl; // Print the minimum level found
    cout << "Max Level: " << max_l << endl; // Print the maximum level found
    cout << "Total Volume (v): " << sum_v << endl; // Print the total volume across all TLs
    cout << "Total Aggressive Buyers (b): " << sum_b << endl; // Print the total volume of aggressive buyers
    cout << "Total Aggressive Sellers (s): " << sum_s << endl; // Print the total volume of aggressive sellers
    // Note: The above values can be used to analyze the distribution of trades across levels
}

void test_trade_to_compare(string symbol) {
    // This function compares the trade data with the TLS data for a given symbol
    utils::Timer timer(symbol); // Create a timer for the operation
    Trades trades(symbol); // Create a Trades object for the symbol
    trades.open(); // Open the binary file for reading trades
    Trade trade; // Create a Trade object to read individual trades
    double sum_v = 0.0; // Initialize sum of volume to 0
    double sum_b = 0.0; // Initialize sum of aggressive buyers to 0
    double sum_s = 0.0; // Initialize sum of aggressive sellers to 0
    for (size_t i = 0; i < trades.count; i++) { // Iterate over each trade in the binary file
        trades.next(trade); // Read the next trade from the binary file
        sum_v += trade.v; // Accumulate the total volume
        if (trade.is_buyer_maker) {
            sum_s += trade.v; // Accumulate the volume of aggressive buyers
        } else {
            sum_b += trade.v; // Accumulate the volume of aggressive sellers
        }
    }
    trades.close(); // Close the binary file after processing all trades
    timer.checkpoint("Completed reading trades"); // Checkpoint after reading all trades
    // print the results
    cout << "Trade Summary for symbol: " << symbol << endl; // Print the summary for the symbol
    cout << "Total Volume (v): " << sum_v << endl; // Print the total volume across all trades
    cout << "Total Aggressive Buyers (b): " << sum_b << endl; // Print the total volume of aggressive buyers
    cout << "Total Aggressive Sellers (s): " << sum_s << endl; // Print the total volume of aggressive sellers
}


int main(int argc, char *argv[]) {
    vector<string> symbols = config.get_csv_strings("symbols_list"); // Get the list of symbols from the configuration
    for (const auto& symbol : symbols) {
        test_tls(symbol); // Test the TLS for each symbol
        test_trade_to_compare(symbol); // Compare the trade data with the TLS data for each symbol
        cout << "----------------------------------------" << endl; // Print a separator for clarity
    }
    return 0;
}

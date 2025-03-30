#include "../../lib/trade/trade.hpp"
#include "../../lib/trade/tl.hpp"
#include "../../lib/config/config.hpp"
#include <iostream>
#include "../../lib/utils/timer.hpp"
#include <vector>
#include <string>



using namespace std;

void trades_to_tls(string symbol) {
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


int main(int argc, char *argv[]) {
    vector<string> symbols = config.get_csv_strings("symbols_list"); // Get the list of symbols from the configuration
    for (const auto& symbol : symbols) {
        trades_to_tls(symbol); // Call the function to process trades to TLS for each symbol
    }
}


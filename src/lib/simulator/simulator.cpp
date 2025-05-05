#include "simulator.hpp"
#include "../trade/trade.hpp"
#include <iostream>


using namespace std;

Simulator::Simulator(string symbol, size_t start_ts, size_t end_ts) {
    this->symbol = symbol;
    this->start_ts = start_ts;
    this->end_ts = end_ts;
}

void Simulator::pre() {
    // Default implementation does nothing
}

Simulator * Simulator::run() {
    this->pre(); // Call the pre method to perform any setup before running the simulation
    Trades trades(this->symbol);
    trades.open(); // Open the binary file for reading trades
    Trade trade;
    size_t start_index = trades.search(this->start_ts); // Find the index of the first trade at or after start_ts
    size_t end_index = trades.search(this->end_ts); // Find the index of the first trade at or after end_ts
    if (start_index == trades.count || start_index >= end_index) {
        cout << "No trades found at or after the specified start timestamp." << endl;
    }
    else {
        trades.set_file_cursor(start_index); // Set the file cursor to the found index
        for (size_t i = start_index; i < end_index; ++i) {
            trades.next(trade); // Read the next trade
            this->new_trade(trade); // Process the trade
        }    
    }
    trades.close(); // Close the binary file after processing
    this->post(); // Call the post method to perform any cleanup after running the simulation
    return this;
}

void Simulator::new_trade(Trade &trade) {
    // Default implementation does nothing
    // This method can be overridden in derived classes to handle new trades
}

void Simulator::post() {
    // Default implementation does nothing
}


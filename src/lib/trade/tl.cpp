#include "tl.hpp"
#include <iostream>
#include "../utils/string_utils.hpp"
#include "../utils/file_utils.hpp"
#include "../utils/datetime_utils.hpp"
#include "../config/config.hpp" // Include config for path settings
#include <fstream> // Include fstream for file operations

using namespace std;


TL::TL() {
    this->t = 0; // Initialize start timestamp
    this->t_end = 0; // Initialize end timestamp
    this->n = 0; // Initialize number of trades
    this->l = 0; // Initialize level
    this->v = 0.0; // Initialize volume
    this->b = 0.0; // Initialize volume of aggressive buyers
    this->s = 0.0; // Initialize volume of aggressive sellers
}

void TL::add_trade(const Trade& trade) {
    if (this->n == 0) this->t = trade.t; // Set start timestamp for the first trade
    this->t_end = trade.t; // Update end timestamp to the latest trade
    this->n++; // Increment the number of trades
    this->v += trade.v; // Update total volume
    if (trade.is_buyer_maker) {
        this->s += trade.v; // Update volume of aggressive buyers
    } else {
        this->b += trade.v; // Update volume of aggressive sellers
    }
}

ostream& operator<<(ostream& os, const TL& tl) {
    // Overload the << operator to print TL objects
    os << "TL(t: " << tl.t << " (" << utils::get_utc_datetime_string(tl.t) << ")"
       << ", t_end: " << tl.t_end << " (" << utils::get_utc_datetime_string(tl.t_end) << ")"
       << ", n: " << tl.n 
       << ", l: " << tl.l 
       << ", v: " << tl.v 
       << ", b: " << tl.b 
       << ", s: " << tl.s 
       << ")";
    return os;
}



TLS::TLS(string symbol) : vector<TL>() {
    this->symbol = utils::toLowerCase(symbol); // Initialize the symbol for this TLS
    string base_path = config.get_path("data_path") + "um/trades/";
    utils::touch_file(base_path + this->symbol + "_levels.bin");
    size_t file_size = utils::get_file_size(base_path + this->symbol + "_levels.bin");
    this->count = file_size / sizeof(TL);
}

void TLS::add_trade(const Trade& trade, size_t l) {
    size_t last_level = rbegin() != rend() ? (rbegin()->l) : 0; // Get the last level if available
    if (l != last_level) {
        emplace_back(); // Create a new TL if the level has changed
        back().l = l; // Set the level for the new TL
        back().add_trade(trade); // Add the trade to the new TL
    } else {
        back().add_trade(trade); // If the level is the same as the last one, add the trade to the existing TL
    }
}

void TLS::import_trades(Trades& trades) {
    PipLevelizer levelizer(symbol); // Create a PipLevelizer for the symbol
    trades.open(); // Open the binary file for reading trades
    trades.set_file_cursor(0); // Set the file cursor to the beginning
    Trade trade;
    for (size_t i = 0; i < trades.count; ++i) {
        trades.next(trade); // Read the next trade from the binary file
        add_trade(trade, levelizer(trade.p)); // Add the trade to the TLS at the appropriate level
    }
    trades.close(); // Close the binary file after processing all trades
}

void TLS::save() {
    string file_path = config.get_path("data_path") + "um/trades/" + symbol + "_levels.bin"; // Define the file path for saving
    ofstream ofs(file_path, ios::binary); // Open the file for writing in binary mode
    if (!ofs.is_open()) {
        cout << "Error: Could not open file for writing: " << file_path << endl;
        return; // Return early if the file cannot be opened
    }
    // write TLS data to file
    ofs.write(reinterpret_cast<const char*>(this->data()), this->size() * sizeof(TL)); // Write the binary data of the TLS to the file
    if (ofs.fail()) {
        cout << "Error: Failed to write to file: " << file_path << endl; // Handle write failure
    } else {
        cout << "Successfully saved TLS data to file: " << file_path << endl; // Success message
    }
    ofs.close(); // Close the file after writing
}

void TLS::open() {
    if (tl_data.is_open()) {
        // cout << "TL data file already open for symbol: " << symbol << endl; // Handle case where file is already open
        return; // Return if the file is already open
    }
    string filename = config.get_path("data_path") + "um/trades/" + symbol + "_levels.bin"; // Define the file path for opening
    tl_data.open(filename, ios::in | ios::binary); // Open the binary file for reading
    if (!tl_data.is_open()) {
        cout << "Error: Could not open TL data file: " << filename << endl; // Handle case where file cannot be opened
        return; // Return early if the file cannot be opened
    }
    // Successfully opened the file
    // cout << "Successfully opened TL data file: " << filename << endl; // Success message
}

void TLS::close() {
    if (tl_data.is_open()) {
        tl_data.close(); // Close the file if it's open
    } else {
        cout << "TL data file for symbol: " << symbol << " is not open." << endl; // Handle case where file is not open
    }
}

void TLS::set_file_cursor(size_t pos) {
    tl_data.seekg(pos * sizeof(TL), ios::beg); // Move to the specified position in the file
}

void TLS::next(TL &tl) {
    tl_data.read(reinterpret_cast<char*>(&tl), sizeof(TL)); // Read the next TL from the binary file
}

bool TLS::read(size_t index, TL &tl) {
    if (index >= this->count) return false; // Return false if the index is out of bounds
    tl_data.seekg(index * sizeof(TL), ios::beg); // Move to the correct position in the file
    tl_data.read(reinterpret_cast<char*>(&tl), sizeof(TL)); // Read the TL data
    return true; // Return true if the TL was successfully read
}

size_t TLS::search(size_t t) {
    size_t left = 0; // Start of the search range
    size_t right = this->count; // End of the search range
    size_t mid = 0; // Middle index for binary search
    TL mid_tl;
    while (left < right - 1) {
        mid = left + (right - left) / 2; // Calculate the middle index
        read(mid, mid_tl); // Read the TL at the middle index
        if (mid_tl.t_end < t) {
            left = mid + 1; // Search in the right half
        } else {
            right = mid; // Search in the left half
        }
    }
    for (mid = left; mid <= right; ++mid) { // Ensure we check the last TL in the range
        if (read(mid, mid_tl)) { // Read the TL at index mid
            if (mid_tl.t_end >= t) { // Check if the end timestamp of the TL is greater than or equal to t
                return mid; // Return the index of the first TL with t_end >= t
            }
        }
    }
    return count;
}

void TLS::read_by_index(size_t start, size_t num) {
    clear(); // Clear the current TLS
    if (start >= this->count) {
        cout << "Error: Start index is out of range." << endl; // Handle out of range error
        return; // Return early if start index is out of range
    }
    if (start + num > this->count) {
        num = this->count - start; // Adjust num to read only available TLs
    }
    resize(num); // Resize the vector to hold the number of TLs to read
    tl_data.seekg(start * sizeof(TL), ios::beg); // Move to the start position in the file
    tl_data.read(reinterpret_cast<char*>(this->data()), num * sizeof(TL)); // Read the TLs into the vector
}


void TLS::read_by_ts(size_t ts1, size_t ts2) {
    clear(); // Clear the current TLS
    if (ts1 >= ts2) {
        cout << "Error: ts1 must be less than ts2." << endl; // Handle invalid timestamp range
        return; // Return early if the timestamp range is invalid
    }
    TL tl;
    size_t index = search(ts1); // Find the starting index for ts1
    size_t end_index = this->search(ts2); // Find the ending index for ts2 (exclusive)
    size_t num = end_index - index + 1; // Calculate the number of TLs to read
    // cout << "Reading TLs from index " << index << " to " << end_index << " (total: " << num << ")" << endl; // Debug output for the range being read
    if (num <= 0) {
        cout << "No TLs found in the specified timestamp range." << endl; // Handle case where no TLs are found
        return; // Return early if no TLs are found
    }
    // Read the TLs from the binary file within the specified timestamp range
    tl_data.seekg(index * sizeof(TL), ios::beg); // Move to the starting position in the file
    resize(num); // Resize the vector to hold the number of TLs to read
    tl_data.read(reinterpret_cast<char*>(this->data()), num * sizeof(TL)); // Read the TLs into the vector
}

TL TLS::first_tl() {
    TL tl;
    this->set_file_cursor(0); // Set cursor to the start
    this->next(tl); // Read the first TL
    return tl; // Return the first TL
}

TL TLS::last_tl() {
    TL tl;
    this->set_file_cursor(count - 1); // Set cursor to the last TL
    this->next(tl); // Read the last TL
    return tl; // Return the last TL
}


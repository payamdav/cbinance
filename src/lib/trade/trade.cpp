#include "trade.hpp"
#include <fstream>
#include "../config/config.hpp"
#include "../utils/string_utils.hpp"
#include "../utils/file_utils.hpp"
#include <charconv>

using namespace std;


Trades::Trades(string symbol) : vector<Trade>() {
    this->symbol = utils::toLowerCase(symbol);
    string base_path = config.get_path("data_path") + "um/trades/";
    utils::touch_file(base_path + this->symbol + ".bin");
    // get file size to determine the number of trades already in the binary file
    size_t file_size = utils::get_file_size(base_path + this->symbol + ".bin");
    this->count = file_size / sizeof(Trade); // Calculate the number of trades in the binary file
}

// operator overload for << to print Trade objects
ostream& operator<<(ostream& os, const Trade& trade) {
    os << "Trade(p: " << trade.p 
       << ", v: " << trade.v 
       << ", q: " << trade.q 
       << ", t: " << trade.t 
       << ", is_buyer_maker: " << (trade.is_buyer_maker ? "true" : "false") 
       << ")";
    return os;
}

void Trades::import_from_csv(int year, int month, int day) {
    this->resize(20000000); // Reserve space for 20 million trades to avoid multiple allocations
    string filename = config.get_path("data_path") + "um/trades/" + utils::toUpperCase(symbol) + "-trades-" + to_string(year) + "-" + utils::lpad(to_string(month), '0', 2) + "-" + utils::lpad(to_string(day), '0', 2) + ".csv";
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file: " << filename << endl;
        return; // Return early if the file cannot be opened
    }
    char * buffer = new char[1024 * 1024 * 1024];
    // read whole file into buffer and set length of read data to buffer_size
    file.read(buffer, 1024 * 1024 * 1024);
    size_t buffer_size = file.gcount();
    file.close();
    // cout << "Read " << buffer_size << " bytes from file: " << filename << endl;
    if (buffer_size == 0) {
        delete[] buffer;
        cout << "Warning: File is empty: " << filename << endl;
        return; // Return early if the file is empty
    }
    size_t i = 0;
    for (; buffer[++i] != '\n';) {}
    i++; // Move past the first line (header)
    size_t n = 0; // Number of trades processed
    size_t token_start;
    while(true) {
        if (i + 20 > buffer_size) { // Check if there's enough data left to read
            break; // Break if not enough data left
        }
        for (; buffer[++i] != ',';) {}
        token_start = i = i+1;
        for (; buffer[++i] != ',';) {}
        from_chars(buffer + token_start, buffer + i, this->at(n).p);
        token_start = i = i+1;
        for (; buffer[++i] != ',';) {}
        from_chars(buffer + token_start, buffer + i, this->at(n).v);
        token_start = i = i+1;
        for (; buffer[++i] != ',';) {}
        from_chars(buffer + token_start, buffer + i, this->at(n).q);
        from_chars(buffer + i + 1, buffer + i + 14, this->at(n).t);
        if (buffer[i+15] == 'f') {
            this->at(n).is_buyer_maker = false;
            i += 21;
        } else  {
            this->at(n).is_buyer_maker = true;
            i += 20;
        }
        n++;
    }
    this->resize(n);

    delete[] buffer;

    // appending to binary file

    string filename2 = config.get_path("data_path") + "um/trades/" + symbol + ".bin";
    ofstream file2(filename2, ios::out | ios::app | ios::binary);
    if (!file2.is_open()) {
        cout << "Error: Could not open file for appending: " << filename2 << endl;
        return; // Return early if the file cannot be opened
    }
    file2.write(reinterpret_cast<char*>(this->data()), this->size() * sizeof(Trade));
    if (file2.fail()) {
        cout << "Error: Failed to write to file: " << filename2 << endl;
    } else {
        cout << "Successfully appended " << this->size() << " trades to file: " << filename2 << endl;
    }
    file2.close();
}

void Trades::open() {
    if (trade_data.is_open()) {
        cout << "Trade data file already open for symbol: " << symbol << endl;
        return; // Return if the file is already open
    }
    string filename = config.get_path("data_path") + "um/trades/" + symbol + ".bin";
    trade_data.open(filename, ios::in | ios::binary); // Open the binary file for reading
    if (!trade_data.is_open()) {
        cout << "Error: Could not open trade data file: " << filename << endl;
        return; // Return early if the file cannot be opened
    }
}

void Trades::close() {
    if (trade_data.is_open()) {
        trade_data.close(); // Close the file if it's open
    } else {
        cout << "Trade data file for symbol: " << symbol << " is not open." << endl;
    }
}

bool Trades::read_trade(size_t index, Trade &trade) {
    if (index >= count) return false;
    trade_data.seekg(index * sizeof(Trade), ios::beg); // Move to the correct position in the file
    trade_data.read(reinterpret_cast<char*>(&trade), sizeof(Trade)); // Read the trade data
    return true; // Return true if the trade was successfully read
}

Trade Trades::read_trade(size_t index) {
    Trade trade;
    if (!read_trade(index, trade)) {
        cout << "Error: Unable to read trade at index: " << index << endl;
        // Handle the error as needed, e.g., return a default trade or throw an exception
    }
    return trade; // Return the trade object
}

Trade Trades::read_first() {
    if (count == 0) {
        throw runtime_error("No trades available to read.");
    }
    return read_trade(0); // Read the first trade
}

Trade Trades::read_last() {
    if (count == 0) {
        throw runtime_error("No trades available to read.");
    }
    return read_trade(count - 1); // Read the last trade
}

size_t Trades::search(size_t t) {
    // Binary search for the trade with timestamp t or greater than t
    size_t left = 0;
    size_t right = count;
    size_t mid;
    Trade mid_trade;
    while (left < right) {
        mid = left + (right - left) / 2; // Calculate the middle index
        read_trade(mid, mid_trade); // Read the trade at the middle index
        if (mid_trade.t < t) {
            left = mid + 1; // Search in the right half
        } else {
            right = mid; // Search in the left half
        }
    }
    return left; // Return the index of the first trade with timestamp >= t    
}

void Trades::read_by_index(size_t start, size_t num) {
    clear();
    // Read trades from start index for num trades
    if (start >= count) {
        cout << "Error: Start index is out of range." << endl;
        return; // Return early if start index is out of range
    }
    if (start + num > count) {
        num = count - start; // Adjust num to read only available trades
    }
    resize(num);
    // Read trades from the binary file
    trade_data.seekg(start * sizeof(Trade), ios::beg); // Move to the start position
    trade_data.read(reinterpret_cast<char*>(this->data()), num * sizeof(Trade)); // Read the trades into the vector
}

void Trades::read_by_ts(size_t ts1, size_t ts2) {
    clear();
    // Read trades within the timestamp range [ts1, ts2]
    if (count == 0 || ts1 > ts2) { // Check if there are no trades or invalid timestamp range
        cout << "No trades available to read." << endl;
        return; // Return early if there are no trades
    }
    size_t start_index = search(ts1); // Find the starting index for ts1
    size_t end_index = search(ts2); // Find the ending index for ts2 (exclusive)
    size_t num = end_index - start_index; // Calculate the number of trades to read
    // cout << "Reading trades from index " << start_index << " to " << end_index << " (total: " << num << ")" << endl;
    if (num == 0) {
        cout << "No trades found in the specified timestamp range." << endl;
        return; // Return early if no trades are found in the range
    }
    resize(num);
    trade_data.seekg(start_index * sizeof(Trade), ios::beg); // Move to the start position
    trade_data.read(reinterpret_cast<char*>(this->data()), num * sizeof(Trade)); // Read the trades into the vector
}


void Trades::min_max_price_from_file_iteration(double &min_price, double &max_price) {
    // Calculate the min and max price from the binary file
    if (count == 0) {
        cout << "No trades available to calculate min/max price." << endl;
        return; // Return early if there are no trades
    }
    min_price = 1e10; // Initialize min_price to a high value
    max_price = 0.0; // Initialize max_price to a low value
    open(); // Open the binary file for reading
    trade_data.seekg(0, ios::beg); // Move to the beginning of the file
    Trade trade;
    for (size_t i = 0; i < count; ++i) {
        // Read each trade from the binary file
        trade_data.read(reinterpret_cast<char*>(&trade), sizeof(Trade)); // Read the trade data
        if (trade.p < min_price) {
            min_price = trade.p; // Update min_price if current trade price is lower
        }
        if (trade.p > max_price) {
            max_price = trade.p; // Update max_price if current trade price is higher
        }
    }
}

void Trades::min_max_price_from_file_iteration_in_chunks(double &min_price, double &max_price) {
    // Calculate the min and max price from the binary file in chunks
    if (count == 0) {
        cout << "No trades available to calculate min/max price." << endl;
        return; // Return early if there are no trades
    }
    min_price = 1e10; // Initialize min_price to a high value
    max_price = 0.0; // Initialize max_price to a low value
    open(); // Open the binary file for reading
    size_t chunk_size = 10000; // Define the chunk size for reading trades
    size_t readed = 0;
    Trade trade;

    while (readed < count) {
        size_t num_to_read = std::min(chunk_size, count - readed); // Calculate number of trades to read in this chunk
        read_by_index(readed, num_to_read); // Read the trades into the vector

        for (const Trade &trade : *this) { // Iterate over the trades in the vector
            if (trade.p < min_price) {
                min_price = trade.p; // Update min_price if current trade price is lower
            }
            if (trade.p > max_price) {
                max_price = trade.p; // Update max_price if current trade price is higher
            }
        }
        readed += num_to_read; // Update the number of trades read
    }

    clear(); // Clear the vector after processing to free memory
    
}


void Trades::set_file_cursor(size_t pos) {
    trade_data.seekg(pos * sizeof(Trade), ios::beg); // Move to the specified position
}

void Trades::next(Trade &trade) {
    trade_data.read(reinterpret_cast<char*>(&trade), sizeof(Trade)); // Read the next trade
}

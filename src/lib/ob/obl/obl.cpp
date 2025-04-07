#include "obl.hpp"
#include <iostream>
#include <string>
#include "../../utils/timer.hpp"
#include "../../utils/datetime_utils.hpp"
#include "../../utils/file_utils.hpp"
#include "../../config/config.hpp"
#include <algorithm>
#include <cstdlib>


obl::OBL::OBL(string symbol) {
    this->symbol = symbol;
    PipLevelizer levelizer(symbol); // Create a levelizer for the symbol to initialize pip levels
    bids.resize(levelizer.levels.size(), 0); // Initialize bids vector with the number of levels
    asks.resize(levelizer.levels.size(), 0); // Initialize asks vector with the number of levels
    string obl_index_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "obl.idx";
    string obl_binary_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "obl.bin";
    ucount = utils::get_file_size(obl_index_path) / OBL_REC_BYTES; // Get the count of records in the index file
    obl_idx.open(obl_index_path, std::ios::binary);
    obl_data.open(obl_binary_path, std::ios::binary);
    t = 0;
    idx = 0; // Initialize the current index in the order book
    std::cout << "OBL constructor called for: " << symbol << std::endl;
}

obl::OBL::~OBL() {
    if (obl_idx.is_open()) {
        obl_idx.close(); // Close the index file if it is open
    }
    if (obl_data.is_open()) {
        obl_data.close(); // Close the binary data file if it is open
    }
    std::cout << "OBL destructor called for: " << symbol << std::endl;
}

obl::OBL * obl::OBL::init(size_t from_ts) {
    idx = find_uid_full(from_ts); // Find the starting uid for the given time
    if (idx >= ucount) {
        std::cerr << "No valid uid found for the given time range" << std::endl;
        return nullptr;
    }
    OBLRec rec;
    if (!get_obl_rec(idx, rec)) {
        std::cerr << "Failed to get initial OBL record at index: " << idx << std::endl;
        return nullptr;
    }
    // Apply the initial record to set up the order book
    apply_obl_rec(rec);
    return this;
}

void obl::OBL::apply_obl_rec(obl::OBLRec &rec) {
    this->t = rec.t; // Set the current time
    if (rec.full) {
        obl_data.seekg(rec.offset, std::ios::beg); // Move to the position in the binary data file
        if (obl_data.fail()) {
            std::cerr << "Failed to seek to the offset in the binary data file" << std::endl;
            exit(1);
        }
        // Read the full snapshot from the binary data file
        obl_data.read(reinterpret_cast<char*>(bids.data()), sizeof(double) * rec.bid_size); // Read bids
        if (obl_data.fail()) {
            std::cerr << "Failed to read bids from binary data" << std::endl;
            exit(1);
        }
        obl_data.read(reinterpret_cast<char*>(asks.data()), sizeof(double) * rec.ask_size); // Read asks
        if (obl_data.fail()) {
            std::cerr << "Failed to read asks from binary data" << std::endl;
            exit(1);
        }
    } else {
        // read the binary data from the obl_data file to fill in the order book
        obl_data.seekg(rec.offset, std::ios::beg);
        for (size_t i = 0; i < rec.bid_size; i++) {
            size_t level;
            double volume;
            obl_data.read(reinterpret_cast<char*>(&level), sizeof(size_t)); // Read level index
            if (obl_data.fail()) {
                std::cerr << "Failed to read level index from binary data" << std::endl;
                exit(1);
            }
            obl_data.read(reinterpret_cast<char*>(&volume), sizeof(double)); // Read volume
            if (obl_data.fail()) {
                std::cerr << "Failed to read volume from binary data" << std::endl;
                exit(1);
            }
            bids[level] = volume; // Set the bid volume at the specified level
        }
        for (size_t i = 0; i < rec.ask_size; i++) {
            size_t level;
            double volume;
            obl_data.read(reinterpret_cast<char*>(&level), sizeof(size_t)); // Read level index
            if (obl_data.fail()) {
                std::cerr << "Failed to read level index from binary data" << std::endl;
                exit(1);
            }
            obl_data.read(reinterpret_cast<char*>(&volume), sizeof(double)); // Read volume
            if (obl_data.fail()) {
                std::cerr << "Failed to read volume from binary data" << std::endl;
                exit(1);
            }
            asks[level] = volume; // Set the ask volume at the specified level
        }
    }
    // Set the first and last levels for bids and asks
    for (size_t i = 0; i < bids.size(); i++) {
        if (bids[i] > eps) {
            fb = i;
            break;
        }
    }
    for (size_t i = bids.size() - 1; i >= 0; i--) {
        if (bids[i] > eps) {
            lb = i;
            break;
        }
    }
    for (size_t i = 0; i < asks.size(); i++) {
        if (asks[i] > eps) {
            fa = i;
            break;
        }
    }
    for (size_t i = asks.size() - 1; i >= 0; i--) {
        if (asks[i] > eps) {
            la = i;
            break;
        }
    }

    after_update(); // Call the after update function to perform any additional processing
    // cout << "Applied OBL record at t: " << t << ", idx: " << idx << std::endl; // Debugging information
    
}

bool obl::OBL::ended() {
    return (idx >= ucount - 1);
}

obl::OBL * obl::OBL::next() {
    OBLRec rec;
    if (ended()) return this;

    idx++; // Move to the next index before applying the record    
    if (!get_obl_rec(idx, rec)) {
        std::cerr << "Failed to get next OBL record at index: " << idx << std::endl;
    }
    else {
        apply_obl_rec(rec);
    }
    return this; // Return the current instance for chaining
    
}

bool obl::OBL::get_obl_rec(size_t idx, obl::OBLRec &rec) {
    obl_idx.seekg(idx * OBL_REC_BYTES, std::ios::beg); // Move to the correct position in the index file
    if (obl_idx.fail()) return false;
    obl_idx.read(reinterpret_cast<char*>(&rec), OBL_REC_BYTES);
    if (obl_idx.fail()) return false;
    return true;
}

size_t obl::OBL::find_uid_full(size_t ts_to_go) {
    if (ts_to_go == 0) return 0; // If ts_to_go is 0, return the first record (the earliest timestamp)
    size_t left = 0;
    size_t right = ucount - 1;
    size_t mid;
    if (ucount == 0) return ucount;
    obl::OBLRec rec;
    while (left < right - 1) {
        mid = left + (right - left) / 2; // Avoid overflow
        if (!get_obl_rec(mid, rec)) return ucount;
        if (rec.t > ts_to_go) right = mid - 1;
        else left = mid;
    }
    for (size_t i = right; i >= 0; i--) { // Ensure we check the last few records
        if (!get_obl_rec(i, rec)) return ucount; // Safety check
        if (rec.full && rec.t <= ts_to_go) {
            // Found the largest record <= ts_to_go and it is full
            return i;
        }
    }
    return ucount; // If no suitable record found, return ucount
}

void obl::OBL::after_update() {
    // cout << "After update called" << endl;
    this->on_after_update(); // Call the virtual function for any additional processing
}


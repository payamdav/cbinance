#include "update.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "../../config/config.hpp"
#include "../../utils/file_utils.hpp"
#include "../../utils/string_utils.hpp"


obl::Update::Update(string symbol) {
    symbol = utils::toLowerCase(symbol);
    this->symbol = symbol;
    // cout << "symbol: " << symbol << endl;
    binary_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "update_level.bin";
    index_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "update_level.idx";
    // cout << "Binary file Path: " << binary_path << endl;
    // cout << "Index file Path: " << index_path << endl;
    if (!utils::is_file_exists(binary_path)) {
        cout << "Binary file not exists: " << binary_path << endl;
        return;
    }
    if (!utils::is_file_exists(index_path)) {
        cout << "Index file not exists: " << index_path << endl;
        return;
    }
}

obl::Update::~Update() {
    this->close();
    // std::cout << "Update: " << symbol << " destroyed" << std::endl;
}

void obl::Update::open() {
    // open if not already open
    if (!update_idx.is_open()) {
        update_idx.open(index_path, std::ios::binary);
        if (!update_idx) {
            std::cerr << "Error opening index file: " << index_path << std::endl;
            return;
        }
    }
    if (!update_data.is_open()) {
        update_data.open(binary_path, std::ios::binary);
        if (!update_data) {
            std::cerr << "Error opening binary file: " << binary_path << std::endl;
            return;
        }
    }
}

void obl::Update::close() {
    if (update_idx.is_open()) {
        update_idx.close();
    }
    if (update_data.is_open()) {
        update_data.close();
    }
}

bool obl::Update::is_sorted_by_t() {
    size_t count = this->count();
    size_t prev_t = 0;
    for (size_t i = 0; i < count; i++) {
        obl::UpdateIdx updateidx = this->get_index(i);
        if (updateidx.t < prev_t) {
            return false;
        }
        prev_t = updateidx.t;
    }
    return true;
}

bool obl::Update::is_sorted_by_u_id() {
    size_t count = this->count();
    size_t prev_u_id = 0;
    for (size_t i = 0; i < count; i++) {
        obl::UpdateIdx updateidx = this->get_index(i);
        if (updateidx.u_id < prev_u_id) {
            return false;
        }
        prev_u_id = updateidx.u_id;
    }
    return true;
}

bool obl::Update::is_pu_aligned() {
    bool is_pu_aligned = true;
    size_t count = this->count();
    obl::UpdateIdx first = this->get_index(0);
    size_t last_u = first.u_id;
    for (size_t i = 1; i < count; i++) {
        obl::UpdateIdx updateidx = this->get_index(i);
        if (updateidx.pu_id != last_u) {
            is_pu_aligned = false;
            cout << "index: " << i << " pu_id: " << updateidx.pu_id << " last_u: " << last_u << endl;
        }
        last_u = updateidx.u_id;
    }
    return is_pu_aligned;
}


bool obl::Update::is_index_size_correct() {
    bool is_index_size_correct = true;
    size_t count = this->count();
    for (size_t i = 0; i < count; i++) {
        obl::UpdateIdx updateidx = this->get_index(i);
        if (updateidx.size != (updateidx.bid_size + updateidx.ask_size) * 2 * sizeof(double)) {
            is_index_size_correct = false;
            cout << "index: " << i << " size: " << updateidx.size << " bid_size: " << updateidx.bid_size << " ask_size: " << updateidx.ask_size << endl;
        }
        if (updateidx.size < 0 || updateidx.size > 5000 * 2 * sizeof(double)) {
            is_index_size_correct = false;
            cout << "index: " << i << " size: " << updateidx.size << " bid_size: " << updateidx.bid_size << " ask_size: " << updateidx.ask_size << endl;
        }
    }
    return is_index_size_correct;
}

bool obl::Update::is_offset_correct() {
    bool is_offset_correct = true;
    size_t count = this->count();
    size_t offset = 0;
    for (size_t i = 0; i < count; i++) {
        obl::UpdateIdx updateidx = this->get_index(i);
        if (updateidx.offset != offset) {
            is_offset_correct = false;
            cout << "index: " << i << " offset: " << updateidx.offset << " expected: " << offset << endl;
        }
        offset += updateidx.size;
    }

    cout << "offset: " << offset << endl;
    return is_offset_correct;
}



size_t obl::Update::count() {
    std::ifstream index_file(index_path, std::ios::binary | std::ios::ate);
    if (!index_file) {
        std::cerr << "Error opening index file: " << index_path << std::endl;
        return 0;
    }
    return index_file.tellg() / sizeof(obl::UpdateIdx); // Calculate the number of entries in the index file
}

obl::UpdateIdx obl::Update::get_index(size_t idx) {
    obl::UpdateIdx updateidx;
    update_idx.seekg(idx * sizeof(obl::UpdateIdx), std::ios::beg);
    update_idx.read((char*)&updateidx, sizeof(obl::UpdateIdx));
    return updateidx;
}

size_t obl::Update::get_index_relevant_to_snapshot(obl::SnapshotIdxLevel sidx, size_t start_index) {
    size_t count = this->count();
    size_t left = start_index;
    size_t right = count - 1;
    size_t mid;
    while (left < right - 1) {
        mid = left + (right - left) / 2;
        obl::UpdateIdx uidx = this->get_index(mid);
        if (uidx.u_id < sidx.u_id) {
            left = mid + 1;
        } else if (uidx.U_id > sidx.u_id) {
            right = mid - 1;
        } else {
            return mid;
        }
    }
    for (mid = left; mid <= right; mid++) {
        obl::UpdateIdx uidx = this->get_index(mid);
        if (uidx.U_id <= sidx.u_id && uidx.u_id >= sidx.u_id) {
            return mid;
        }
    }
    return count; // not found
}

void obl::Update::get_update(const obl::UpdateIdx& updateidx, unordered_map<size_t, double>& b, unordered_map<size_t, double>& a) {
    b.clear(); // Clear the existing data in the maps
    a.clear(); // Clear the existing data in the maps
    update_data.seekg(updateidx.offset, std::ios::beg);
    for (size_t i = 0; i < updateidx.bid_size; i++) {
        size_t level;
        double vol;
        update_data.read((char*)&level, sizeof(size_t)); // Read level (size_t)
        update_data.read((char*)&vol, sizeof(double)); // Read volume (double)
        b[level] = vol; // Store in the bids map
    }
    for (size_t i = 0; i < updateidx.ask_size; i++) {
        size_t level;
        double vol;
        update_data.read((char*)&level, sizeof(size_t)); // Read level (size_t)
        update_data.read((char*)&vol, sizeof(double)); // Read volume (double)
        a[level] = vol; // Store in the asks map
    }

}


std::ostream& obl::operator<<(std::ostream& os, const obl::UpdateIdx& update_idx) {
    os << "t: " << update_idx.t << ", U_id: " << update_idx.U_id << ", u_id: " << update_idx.u_id
       << ", pu_id: " << update_idx.pu_id << ", offset: " << update_idx.offset
       << ", size: " << update_idx.size << ", bid_size: " << update_idx.bid_size
       << ", ask_size: " << update_idx.ask_size;
    return os;
}
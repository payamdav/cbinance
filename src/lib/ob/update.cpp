#include "update.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "../config/config.hpp"
#include "../utils/file_utils.hpp"
#include "../utils/string_utils.hpp"


Update::Update(string symbol) {
    symbol = utils::toLowerCase(symbol);
    this->symbol = symbol;
    cout << "symbol: " << symbol << endl;
    binary_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "update.bin";
    index_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "update.idx";
    cout << "Binary file Path: " << binary_path << endl;
    cout << "Index file Path: " << index_path << endl;
    if (!utils::is_file_exists(binary_path)) {
        cout << "Binary file not exists: " << binary_path << endl;
        return;
    }
    if (!utils::is_file_exists(index_path)) {
        cout << "Index file not exists: " << index_path << endl;
        return;
    }
}

Update::~Update() {
    this->close();
    std::cout << "Update: " << symbol << " destroyed" << std::endl;
}

void Update::open() {
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

void Update::close() {
    if (update_idx.is_open()) {
        update_idx.close();
    }
    if (update_data.is_open()) {
        update_data.close();
    }
}

bool Update::is_sorted_by_t() {
    size_t count = this->count();
    size_t prev_t = 0;
    for (size_t i = 0; i < count; i++) {
        UpdateIdx updateidx = this->get_index(i);
        if (updateidx.t < prev_t) {
            return false;
        }
        prev_t = updateidx.t;
    }
    return true;
}

bool Update::is_sorted_by_u_id() {
    size_t count = this->count();
    size_t prev_u_id = 0;
    for (size_t i = 0; i < count; i++) {
        UpdateIdx updateidx = this->get_index(i);
        if (updateidx.u_id < prev_u_id) {
            return false;
        }
        prev_u_id = updateidx.u_id;
    }
    return true;
}

bool Update::is_pu_aligned() {
    bool is_pu_aligned = true;
    size_t count = this->count();
    UpdateIdx first = this->get_index(0);
    size_t last_u = first.u_id;
    for (size_t i = 1; i < count; i++) {
        UpdateIdx updateidx = this->get_index(i);
        if (updateidx.pu_id != last_u) {
            is_pu_aligned = false;
            cout << "index: " << i << " pu_id: " << updateidx.pu_id << " last_u: " << last_u << endl;
        }
        last_u = updateidx.u_id;
    }
    return is_pu_aligned;
}


bool Update::is_index_size_correct() {
    bool is_index_size_correct = true;
    size_t count = this->count();
    for (size_t i = 0; i < count; i++) {
        UpdateIdx updateidx = this->get_index(i);
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

bool Update::is_offset_correct() {
    bool is_offset_correct = true;
    size_t count = this->count();
    size_t offset = 0;
    for (size_t i = 0; i < count; i++) {
        UpdateIdx updateidx = this->get_index(i);
        if (updateidx.offset != offset) {
            is_offset_correct = false;
            cout << "index: " << i << " offset: " << updateidx.offset << " expected: " << offset << endl;
        }
        offset += updateidx.size;
    }

    cout << "offset: " << offset << endl;
    return is_offset_correct;
}



size_t Update::count() {
    std::ifstream index_file(index_path, std::ios::binary | std::ios::ate);
    if (!index_file) {
        std::cerr << "Error opening index file: " << index_path << std::endl;
        return 0;
    }
    return index_file.tellg() / UPDATE_IDX_BYTES;
}

UpdateIdx Update::get_index(size_t idx) {
    UpdateIdx updateidx;
    update_idx.seekg(idx * UPDATE_IDX_BYTES, std::ios::beg);
    update_idx.read((char*)&updateidx, UPDATE_IDX_BYTES);
    return updateidx;
}

size_t Update::get_index_relevant_to_snapshot(SnapshotIdx sidx, size_t start_index) {
    size_t count = this->count();
    size_t left = start_index;
    size_t right = count - 1;
    size_t mid;
    while (left < right) {
        mid = left + (right - left) / 2;
        UpdateIdx uidx = this->get_index(mid);
        if (uidx.u_id < sidx.u_id) {
            left = mid + 1;
        } else if (uidx.U_id > sidx.u_id) {
            right = mid - 1;
        } else {
            return mid;
        }
    }
    return count; // not found
}

void Update::get_update(const UpdateIdx& updateidx, vector<double>& bp, vector<double>& bv, vector<double>& ap, vector<double>& av) {
    bp.clear();
    bv.clear();
    ap.clear();
    av.clear();
    update_data.seekg(updateidx.offset, std::ios::beg);
    if (updateidx.bid_size > 0) {
        update_data.read((char*)bp.data(), updateidx.bid_size * sizeof(double));
        update_data.read((char*)bv.data(), updateidx.bid_size * sizeof(double));
    }
    if (updateidx.ask_size > 0) {
        update_data.read((char*)ap.data(), updateidx.ask_size * sizeof(double));
        update_data.read((char*)av.data(), updateidx.ask_size * sizeof(double));
    }
}

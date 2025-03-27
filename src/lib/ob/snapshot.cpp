#include <iostream>
#include <fstream>
#include <string>
#include "snapshot.hpp"
#include "../config/config.hpp"
#include "../utils/file_utils.hpp"
#include "../utils/string_utils.hpp"



Snapshot::Snapshot(string symbol) {
    symbol = utils::toLowerCase(symbol);
    this->symbol = symbol;
    cout << "symbol: " << symbol << endl;
    binary_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "snapshot.bin";
    index_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "snapshot.idx";
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

Snapshot::~Snapshot() {
    std::cout << "Snapshot: " << symbol << " destroyed" << std::endl;
}

size_t Snapshot::count() {
    std::ifstream index_file(index_path, std::ios::binary | std::ios::ate);
    if (!index_file) {
        std::cerr << "Error opening index file: " << index_path << std::endl;
        return 0;
    }
    return index_file.tellg() / SNAPSHOT_IDX_BYTES;
}

void Snapshot::open() {
    // open if not already open
    if (!snapshot_idx.is_open()) {
        snapshot_idx.open(index_path, std::ios::binary);
        if (!snapshot_idx) {
            std::cerr << "Error opening index file: " << index_path << std::endl;
            return;
        }
    }
    if (!snapshot_data.is_open()) {
        snapshot_data.open(binary_path, std::ios::binary);
        if (!snapshot_data) {
            std::cerr << "Error opening binary file: " << binary_path << std::endl;
            return;
        }
    }
}

void Snapshot::close() {
    if (snapshot_idx.is_open()) snapshot_idx.close();
    if (snapshot_data.is_open()) snapshot_data.close();
    snapshot_idx.close();
    snapshot_data.close();
}

bool Snapshot::is_sorted_by_t() {
    size_t count = this->count();
    size_t prev_t = 0;
    for (size_t i = 0; i < count; i++) {
        SnapshotIdx snapshotidx = get_index(i);
        if (snapshotidx.t < prev_t) {
            return false;
        }
        prev_t = snapshotidx.t;
    }
    return true;
}

bool Snapshot::is_sorted_by_u_id() {
    size_t count = this->count();
    size_t prev_u_id = 0;
    for (size_t i = 0; i < count; i++) {
        SnapshotIdx snapshotidx = get_index(i);
        if (snapshotidx.u_id < prev_u_id) {
            return false;
        }
        prev_u_id = snapshotidx.u_id;
    }
    return true;
}

SnapshotIdx Snapshot::get_index(size_t idx) {
    snapshot_idx.seekg(idx * SNAPSHOT_IDX_BYTES, std::ios::beg);
    SnapshotIdx snapshotidx;
    snapshot_idx.read(reinterpret_cast<char*>(&snapshotidx), SNAPSHOT_IDX_BYTES);
    return snapshotidx;
}

size_t Snapshot::get_index_gte(size_t t) {
    size_t count = this->count();
    size_t left = 0;
    size_t right = count - 1;
    size_t mid;
    while (left < right) {
        mid = left + (right - left) / 2;
        SnapshotIdx snapshotidx = get_index(mid);
        if (snapshotidx.t < t) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

void Snapshot::get_snapshot(const SnapshotIdx& snapshotidx, vector<double>& bp, vector<double>& bv, vector<double>& ap, vector<double>& av) {
    snapshot_data.seekg(snapshotidx.offset, std::ios::beg);
    bp.clear();
    bv.clear();
    ap.clear();
    av.clear();
    snapshot_data.read(reinterpret_cast<char*>(bp.data()), snapshotidx.bid_size * sizeof(double));
    snapshot_data.read(reinterpret_cast<char*>(bv.data()), snapshotidx.bid_size * sizeof(double));
    snapshot_data.read(reinterpret_cast<char*>(ap.data()), snapshotidx.ask_size * sizeof(double));
    snapshot_data.read(reinterpret_cast<char*>(av.data()), snapshotidx.ask_size * sizeof(double));
}
#include <iostream>
#include <fstream>
#include <string>
#include "snapshot.hpp"
#include "../../config/config.hpp"
#include "../../utils/file_utils.hpp"
#include "../../utils/string_utils.hpp"



obl::Snapshot::Snapshot(string symbol) {
    symbol = utils::toLowerCase(symbol);
    this->symbol = symbol;
    // cout << "symbol: " << symbol << endl;
    binary_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "snapshot_level.bin";
    index_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "snapshot_level.idx";
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

obl::Snapshot::~Snapshot() {
    this->close();
    // std::cout << "Snapshot: " << symbol << " destroyed" << std::endl;
}

size_t obl::Snapshot::count() {
    std::ifstream index_file(index_path, std::ios::binary | std::ios::ate);
    if (!index_file) {
        std::cerr << "Error opening index file: " << index_path << std::endl;
        return 0;
    }
    return index_file.tellg() / sizeof(SnapshotIdxLevel); // Calculate the number of entries in the index file
}

void obl::Snapshot::open() {
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

void obl::Snapshot::close() {
    if (snapshot_idx.is_open()) snapshot_idx.close();
    if (snapshot_data.is_open()) snapshot_data.close();
    snapshot_idx.close();
    snapshot_data.close();
}

bool obl::Snapshot::is_sorted_by_t() {
    size_t count = this->count();
    size_t prev_t = 0;
    for (size_t i = 0; i < count; i++) {
        SnapshotIdxLevel snapshotidx = get_index(i);
        if (snapshotidx.t < prev_t) {
            return false;
        }
        prev_t = snapshotidx.t;
    }
    return true;
}

bool obl::Snapshot::is_sorted_by_u_id() {
    size_t count = this->count();
    size_t prev_u_id = 0;
    for (size_t i = 0; i < count; i++) {
        SnapshotIdxLevel snapshotidx = get_index(i);
        if (snapshotidx.u_id < prev_u_id) {
            return false;
        }
        prev_u_id = snapshotidx.u_id;
    }
    return true;
}

obl::SnapshotIdxLevel obl::Snapshot::get_index(size_t idx) {
    snapshot_idx.seekg(idx * sizeof(SnapshotIdxLevel), std::ios::beg);
    SnapshotIdxLevel snapshotidx;
    snapshot_idx.read(reinterpret_cast<char*>(&snapshotidx), sizeof(SnapshotIdxLevel)); // Read the index entry from the file
    return snapshotidx;
}

size_t obl::Snapshot::get_index_gte(size_t t) {
    size_t count = this->count();
    size_t left = 0;
    size_t right = count - 1;
    size_t mid;
    while (left < right - 1) {
        mid = left + (right - left) / 2;
        SnapshotIdxLevel snapshotidx = get_index(mid);
        if (snapshotidx.t < t) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    for (mid = left; mid <= right; mid++) {
        SnapshotIdxLevel snapshotidx = get_index(mid);
        if (snapshotidx.t >= t) {
            return mid;
        }
    }
    return count;
}

void obl::Snapshot::get_snapshot(const SnapshotIdxLevel& snapshotidx, vector<double>& b, vector<double>& a) {
    b.resize(snapshotidx.last_bid_level - snapshotidx.first_bid_level + 1 );
    a.resize(snapshotidx.last_ask_level - snapshotidx.first_ask_level + 1 );
    snapshot_data.seekg(snapshotidx.offset, std::ios::beg);
    snapshot_data.read(reinterpret_cast<char*>(b.data()), b.size() * sizeof(double));
    snapshot_data.read(reinterpret_cast<char*>(a.data()), a.size() * sizeof(double));
}

std::ostream& obl::operator<<(std::ostream& os, const obl::SnapshotIdxLevel& snapshot_idx) {
    os << "Time: " << snapshot_idx.t << ", Unique ID: " << snapshot_idx.u_id
       << ", First Bid Level: " << snapshot_idx.first_bid_level
       << ", Last Bid Level: " << snapshot_idx.last_bid_level
       << ", First Ask Level: " << snapshot_idx.first_ask_level
       << ", Last Ask Level: " << snapshot_idx.last_ask_level;
    return os;
}
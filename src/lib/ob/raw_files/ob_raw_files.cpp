#include "ob_raw_files.hpp"
#include <iostream>
#include "../../config/config.hpp"
#include "../../utils/string_utils.hpp"
#include "../../utils/file_utils.hpp"
#include <filesystem>
#include <string_view>
#include <charconv>



using namespace std;


ob::Symbol::Symbol(string symbol) : snapshot_last_u_id(0), update_last_u_id(0) {
    this->symbol = utils::toLowerCase(utils::trim(symbol));
    cout << "Symbol constructor called for: " << this->symbol << endl;
    base_path = config.get_path("data_path") + "um/depth/" + this->symbol + "/";
    filesystem::create_directories(base_path);
    utils::touch_file(base_path + "snapshot.bin");
    utils::touch_file(base_path + "snapshot.idx");
    utils::touch_file(base_path + "update.bin");
    utils::touch_file(base_path + "update.idx");
    get_last_update_id();
    // open files for appending
    snapshot_data.open(base_path + "snapshot.bin", ios::out | ios::app | ios::binary);
    snapshot_idx.open(base_path + "snapshot.idx", ios::out | ios::app | ios::binary);
    update_data.open(base_path + "update.bin", ios::out | ios::app | ios::binary);
    update_idx.open(base_path + "update.idx", ios::out | ios::app | ios::binary);
    if (!snapshot_data.is_open() || !snapshot_idx.is_open() || !update_data.is_open() || !update_idx.is_open()) {
        cout << "Error opening files for symbol: " << this->symbol << endl;
        exit(1);
    }
    cout << "Files opened for symbol: " << this->symbol << endl;
}

void ob::Symbol::get_last_update_id() {
    // get last update id from snapshot.idx
    size_t snapshot_idx_size = utils::get_file_size(base_path + "snapshot.idx");
    if (snapshot_idx_size > 0) {
        ifstream f(base_path + "snapshot.idx", ios::in | ios::binary);
        if (f.is_open()) {
            f.seekg(-SNAPSHOT_IDX_BYTES, ios::end);
            SnapshotIdx snapshot_idx;
            f.read(reinterpret_cast<char*>(&snapshot_idx), sizeof(SnapshotIdx));
            snapshot_last_u_id = snapshot_idx.u_id;
            f.close();
            cout << "Last snapshot update id: " << snapshot_last_u_id << endl;
        } else {
            cout << "Error opening snapshot.idx file" << endl;
        }
    }

    // get last update id from update.idx
    size_t update_idx_size = utils::get_file_size(base_path + "update.idx");
    if (update_idx_size > 0) {
        ifstream f(base_path + "update.idx", ios::in | ios::binary);
        if (f.is_open()) {
            f.seekg(-UPDATE_IDX_BYTES, ios::end);
            UpdateIdx update_idx;
            f.read(reinterpret_cast<char*>(&update_idx), sizeof(UpdateIdx));
            update_last_u_id = update_idx.u_id;
            f.close();
            cout << "Last update id: " << update_last_u_id << endl;
        } else {
            cout << "Error opening update.idx file" << endl;
        }
    }
}

ob::Symbol::~Symbol() {
    cout << "Symbol destructor called for: " << this->symbol << endl;
    if (snapshot_data.is_open()) {
        snapshot_data.close();
    }
    if (snapshot_idx.is_open()) {
        snapshot_idx.close();
    }
    if (update_data.is_open()) {
        update_data.close();
    }
    if (update_idx.is_open()) {
        update_idx.close();
    }
}

ostream& ob::operator<<(ostream& os, const ob::SnapshotIdx& snapshot_idx) {
    os << "t: " << snapshot_idx.t << ", u_id: " << snapshot_idx.u_id << ", offset: " << snapshot_idx.offset << ", size: " << snapshot_idx.size << ", bid_size: " << snapshot_idx.bid_size << ", ask_size: " << snapshot_idx.ask_size;
    return os;
}

ostream& ob::operator<<(ostream& os, const ob::UpdateIdx& update_idx) {
    os << "t: " << update_idx.t << ", U_id: " << update_idx.U_id << ", u_id: " << update_idx.u_id << ", pu_id: " << update_idx.pu_id << ", offset: " << update_idx.offset << ", size: " << update_idx.size << ", bid_size: " << update_idx.bid_size << ", ask_size: " << update_idx.ask_size;
    return os;
}


ob::ObRawFiles::ObRawFiles() {
    // cout << "ObRawFiles constructor called" << endl;
    vector<string> symbols = config.get_csv_strings("symbols_list");
    string data_path = config.get_path("data_path");
    snapshot_raw_path = data_path + "queuewriter/depth_snapshot/";
    update_raw_path = data_path + "queuewriter/depth_update/";
    for (const auto& symbol : symbols) {
        Symbol* sym = new Symbol(symbol);
        symbols_map[sym->symbol] = sym;
    }
}

ob::ObRawFiles::~ObRawFiles() {
    // cout << "ObRawFiles destructor called" << endl;
    for (auto& pair : symbols_map) {
        delete pair.second;
    }
    symbols_map.clear();
}


void ob::ObRawFiles::import_snapshot(int file_id_from, int file_id_to) {
    if (file_id_to == -1) file_id_to = file_id_from;
    string snapshot_file;
    char* buffer = new char[200 * 1024 * 1024];
    char* sbuf;
    size_t buffer_size;
    size_t i;
    SnapshotIdx snapshot_idx;
    string symbol;
    size_t token_start;
    double prices[10000];
    double vols[10000];
    size_t n;
    bool is_price;
    size_t snapshot_count = 0;


    for (int file_id = file_id_from; file_id <= file_id_to; file_id++) {

        snapshot_file = snapshot_raw_path + "depth_snapshot-" + utils::lpad(to_string(file_id), '0', 10) + ".json";
        cout << "Snapshot file: " << snapshot_file << endl;
        ifstream f(snapshot_file);
        if (!f.is_open()) {
            cout << "Error opening snapshot file: " << snapshot_file << endl;
            return;
        }
        f.read(buffer, 200 * 1024 * 1024);
        buffer_size = f.gcount();
        f.close();
        cout << "Buffer size: " << buffer_size << endl;
        i = 0;

        sbuf = buffer;

        while(true) {

            sbuf = sbuf + i;
            if (sbuf - buffer + 10 >= buffer_size) break;

            snapshot_count++;

            i = 11;

            for (;; i++) if (sbuf[i] == '\"') break;
            symbol = string(sbuf + 11, i - 11);

            token_start = i + 25;
            for (i=token_start; ; i++) if (sbuf[i] == ',') break;
            from_chars(sbuf + token_start, sbuf + i, snapshot_idx.u_id);

            from_chars(sbuf + i + 23, sbuf + i + 36, snapshot_idx.t);

            i += 47;
            // bids
            n = 0;
            is_price = true;
            token_start = i;
            for (; ; i++) {
                if (sbuf[i] == 'k') break;
                if (sbuf[i] == '\"') {
                    if (is_price) {
                        from_chars(sbuf + token_start, sbuf + i, prices[n]);
                        token_start = i + 3;
                    } else {
                        from_chars(sbuf + token_start, sbuf + i, vols[n]);
                        n++;
                        token_start = i + 5;
                    }
                    is_price = !is_price;
                    i = token_start;
                }
            }

            snapshot_idx.bid_size = n;
            if (snapshot_idx.u_id > symbols_map[symbol]->snapshot_last_u_id) {
                snapshot_idx.offset = symbols_map[symbol]->snapshot_data.tellp();
                symbols_map[symbol]->snapshot_data.write(reinterpret_cast<char*>(prices), n * sizeof(double));
                symbols_map[symbol]->snapshot_data.write(reinterpret_cast<char*>(vols), n * sizeof(double));
                snapshot_idx.size = n * 2 * sizeof(double);
            }

            // asks
            n = 0;
            is_price = true;
            i += 7;
            token_start = i;
            for (; ; i++) {
                if (sbuf[i] == '\"') {
                    if (is_price) {
                        from_chars(sbuf + token_start, sbuf + i, prices[n]);
                        token_start = i + 3;
                    } else {
                        from_chars(sbuf + token_start, sbuf + i, vols[n]);
                        n++;
                        token_start = i + 5;
                        if (sbuf[i+2] == ']') break;
                    }
                    is_price = !is_price;
                    i = token_start;
                }
            }

            snapshot_idx.ask_size = n;
            if (snapshot_idx.u_id > symbols_map[symbol]->snapshot_last_u_id) {
                symbols_map[symbol]->snapshot_data.write(reinterpret_cast<char*>(prices), n * sizeof(double));
                symbols_map[symbol]->snapshot_data.write(reinterpret_cast<char*>(vols), n * sizeof(double));
                snapshot_idx.size += (n * 2 * sizeof(double));
                symbols_map[symbol]->snapshot_idx.write(reinterpret_cast<char*>(&snapshot_idx), sizeof(SnapshotIdx));
                symbols_map[symbol]->snapshot_last_u_id = snapshot_idx.u_id;
            }

            // cout << "Symbol: " << symbol << ", ts: " << snapshot_idx.t << ", u_id: " << snapshot_idx.u_id << ", bids_count: " << snapshot_idx.bid_size << ", asks_count: " << snapshot_idx.ask_size << endl;
            i += 6;

        
        }


        cout << "Snapshot count: " << snapshot_count << endl;
    }


}


void ob::ObRawFiles::import_update(int file_id_from, int file_id_to) {
    if (file_id_to == -1) file_id_to = file_id_from;
    string update_file;
    char* buffer = new char[200 * 1024 * 1024];
    char* sbuf;
    size_t buffer_size;
    size_t i;
    UpdateIdx update_idx;
    string symbol;
    size_t token_start;
    double prices[10000];
    double vols[10000];
    size_t n;
    bool is_price;
    size_t update_count = 0;
    bool no_bid = false;
    bool no_ask = false;


    for (int file_id = file_id_from; file_id <= file_id_to; file_id++) {

        update_file = update_raw_path + "depth_update-" + utils::lpad(to_string(file_id), '0', 10) + ".json";
        cout << "Update file: " << update_file << endl;
        ifstream f(update_file);
        if (!f.is_open()) {
            cout << "Error opening update file: " << update_file << endl;
            return;
        }
        f.read(buffer, 200 * 1024 * 1024);
        buffer_size = f.gcount();
        f.close();
        cout << "Buffer size: " << buffer_size << endl;
        i = 0;

        sbuf = buffer;

        while(true) {

            sbuf = sbuf + i;
            if (sbuf - buffer + 10 >= buffer_size) break;

            update_count++;
            no_bid = false;
            no_ask = false;

            i = 1;

            for (;; i++) if (sbuf[i] == 'T') break;
            from_chars(sbuf + i + 3, sbuf + i + 16, update_idx.t);

            token_start = i + 22;
            for (i=token_start;; i++) if (sbuf[i] == '\"') break;
            symbol = utils::toLowerCase(string(sbuf + token_start, i - token_start));

            token_start = i + 6;
            for (i=token_start;; i++) if (sbuf[i] == ',') break;
            from_chars(sbuf + token_start, sbuf + i, update_idx.U_id);

            token_start = i + 5;
            for (i=token_start;; i++) if (sbuf[i] == ',') break;
            from_chars(sbuf + token_start, sbuf + i, update_idx.u_id);

            token_start = i + 6;
            for (i=token_start;; i++) if (sbuf[i] == ',') break;
            from_chars(sbuf + token_start, sbuf + i, update_idx.pu_id);

            update_idx.offset = symbols_map[symbol]->update_data.tellp();

            if (sbuf[i+6] == ']') no_bid = true;
            if (!no_bid) {
                // bids
                i += 8;
                n = 0;
                is_price = true;
                token_start = i;
                for (; ; i++) {
                    if (sbuf[i] == '\"') {
                        if (is_price) {
                            from_chars(sbuf + token_start, sbuf + i, prices[n]);
                            token_start = i + 3;
                        } else {
                            from_chars(sbuf + token_start, sbuf + i, vols[n]);
                            n++;
                            token_start = i + 5;
                            if (sbuf[i+2] == ']') break;
                        }
                        is_price = !is_price;
                        i = token_start;
                    }
                }

                update_idx.bid_size = n;
                if (update_idx.u_id > symbols_map[symbol]->update_last_u_id) {                    
                    symbols_map[symbol]->update_data.write(reinterpret_cast<char*>(prices), n * sizeof(double));
                    symbols_map[symbol]->update_data.write(reinterpret_cast<char*>(vols), n * sizeof(double));
                    update_idx.size = n * 2 * sizeof(double);
                }
                i += 3;
            } else {
                update_idx.bid_size = 0;
                update_idx.size = 0;
                i += 7;
            }

            if (sbuf[i+6] == ']') no_ask = true;
            if (!no_ask) {
                // asks
                n = 0;
                is_price = true;
                i += 8;
                token_start = i;
                for (; ; i++) {
                    if (sbuf[i] == '\"') {
                        if (is_price) {
                            from_chars(sbuf + token_start, sbuf + i, prices[n]);
                            token_start = i + 3;
                        } else {
                            from_chars(sbuf + token_start, sbuf + i, vols[n]);
                            n++;
                            token_start = i + 5;
                            if (sbuf[i+2] == ']') break;
                        }
                        is_price = !is_price;
                        i = token_start;
                    }
                }

                update_idx.ask_size = n;
                if (update_idx.u_id > symbols_map[symbol]->update_last_u_id) {
                    symbols_map[symbol]->update_data.write(reinterpret_cast<char*>(prices), n * sizeof(double));
                    symbols_map[symbol]->update_data.write(reinterpret_cast<char*>(vols), n * sizeof(double));
                    update_idx.size += (n * 2 * sizeof(double));
                    symbols_map[symbol]->update_idx.write(reinterpret_cast<char*>(&update_idx), sizeof(UpdateIdx));
                    symbols_map[symbol]->update_last_u_id = update_idx.u_id;
                }
                i += 6;
            } else {
                update_idx.ask_size = 0;
                if (update_idx.u_id > symbols_map[symbol]->update_last_u_id) {
                    symbols_map[symbol]->update_idx.write(reinterpret_cast<char*>(&update_idx), sizeof(UpdateIdx));
                    symbols_map[symbol]->update_last_u_id = update_idx.u_id;
                }
                i += 10;
            }

            // cout << "Symbol: " << symbol << ", ts: " << update_idx.t << ", U_id: " << update_idx.U_id << ", u_id: " << update_idx.u_id << ", pu_id: " << update_idx.pu_id << ", bids_count: " << update_idx.bid_size << ", asks_count: " << update_idx.ask_size << endl;

        }


        cout << "Update count: " << update_count << endl;

    }


}

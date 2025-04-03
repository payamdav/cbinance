#include "obl_creator.hpp"
#include <fstream>
#include <iostream>
#include "../../config/config.hpp"
#include <string>


using namespace std;


ob::OBLC::OBLC(string symbol) : ob::OB(symbol), levelizer(symbol) {
    string updates_level_index_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "update_level.idx";
    string updates_level_binary_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "update_level.bin";
    // open files for appending
    update_idx.open(updates_level_index_path, std::ios::binary | std::ios::app);
    update_data.open(updates_level_binary_path, std::ios::binary | std::ios::app);
}

void ob::OBLC::build(size_t from_ts, size_t to_ts) {
    // cout << "Building OB for symbol: " << symbol << endl;
    size_t ts_to_go = from_ts;
    size_t sid;
    size_t uid;

    while (ts_to_go < to_ts) {
        if (!find_sid_uid(ts_to_go, sid, uid)) {
            // cout << "No snapshot with relevant update found for ts: " << ts_to_go << endl;
            return;
        }
        bids.clear();
        asks.clear();

        SnapshotIdx sidx = snapshot->get_index(sid);
        apply_snapshot(sidx);
        UpdateIdx uidx = update->get_index(uid);
        apply_update(uidx);

        while(true) {
            uid++;
            if (uid >= ucount) {
                // cout << "No more updates to apply for ts: " << ts_to_go << endl;
                return;
            }
            uidx = update->get_index(uid);
            if (uidx.pu_id != u_id) {
                cout << "update gap detected. at uid: " << uid << " - current u_id: " << u_id << " - uidx: " << uidx << endl;
                write_gap_update_to_file(uidx);
                ts_to_go = uidx.t;
                break;
            }
            apply_update(uidx);

        }

    }
    
}

void ob::OBLC::apply_price_vol_update(const ob::UpdateIdx & uidx) {
    double current_vol = 0;
    double delta;
    size_t l;
    bool is_in_range;
    diff_bids.clear();
    diff_asks.clear();

    for (size_t i = 0; i < bp.size(); i++) {
        is_in_range = levelizer.is_in_range(bp[i]); // Check if the price is within the range defined by PipLevelizer
        l = is_in_range ? levelizer.get_level(bp[i]) : 0; // Get the level for the price if in range
        current_vol = bids.contains(bp[i]) ? bids[bp[i]] : 0; // Get the current volume for this price
        delta = bv[i] - current_vol; // Calculate the difference between new volume and current volume
        if (is_in_range && delta != 0) {
            if (diff_bids.contains(l)) {
                diff_bids[l] += delta;
            } else {
                diff_bids[l] = delta;
            }
        }

        if (bv[i] == 0) {
            bids.erase(bp[i]);
        } else {
            bids[bp[i]] = bv[i];
        }

    }

    for (size_t i = 0; i < ap.size(); i++) {
        is_in_range = levelizer.is_in_range(ap[i]); // Check if the price is within the range defined by PipLevelizer
        l = is_in_range ? levelizer.get_level(ap[i]) : 0; // Get the level for the price if in range
        current_vol = asks.contains(ap[i]) ? asks[ap[i]] : 0; // Get the current volume for this price
        delta = av[i] - current_vol; // Calculate the difference between new volume and current volume
        if (is_in_range && delta != 0) {
            if (diff_asks.contains(l)) {
                diff_asks[l] += delta;
            } else {
                diff_asks[l] = delta;
            }
        }

        if (av[i] == 0) {
            asks.erase(ap[i]);
        } else {
            asks[ap[i]] = av[i];
        }
    }

    write_diff_to_file(uidx); // Write the differences to file for further processing or analysis


}


void ob::OBLC::write_diff_to_file(const ob::UpdateIdx & uidxin) {
    UpdateIdx uidx = uidxin; // Copy the input UpdateIdx to a local variable for processing
    uidx.bid_size = diff_bids.size(); // Set the bid size in the UpdateIdx based on the diff_bids map
    uidx.ask_size = diff_asks.size(); // Set the ask size in the UpdateIdx based on the diff_asks map
    uidx.size = (uidx.bid_size + uidx.ask_size) * (sizeof(double) + sizeof(size_t)); // Calculate the size of the update based on bid and ask sizes
    uidx.offset = update_data.tellp(); // Get the current position in the update data file for the offset
    // Write the UpdateIdx to the index file
    update_idx.write((char*)&uidx, sizeof(UpdateIdx)); // Write the UpdateIdx structure to the index file
    // Write the differences for bids to the update data file
    for (const auto& pair : diff_bids) {
        update_data.write((char*)&pair.first, sizeof(size_t)); // Write the level (size_t) for the bid
        update_data.write((char*)&pair.second, sizeof(double)); // Write the volume (double) for the bid
    }
    // Write the differences for asks to the update data file
    for (const auto& pair : diff_asks) {
        update_data.write((char*)&pair.first, sizeof(size_t)); // Write the level (size_t) for the ask
        update_data.write((char*)&pair.second, sizeof(double)); // Write the volume (double) for the ask
    }
}

void ob::OBLC::write_gap_update_to_file(const ob::UpdateIdx & uidxin) {
    UpdateIdx uidx = uidxin; // Copy the input UpdateIdx to a local variable for processing
    uidx.bid_size = 0; // Set the bid size in the UpdateIdx to 0 for gap updates
    uidx.ask_size = 0; // Set the ask size in the UpdateIdx to 0 for gap updates
    uidx.size = 0; // Set the size of the update to 0 for gap updates
    uidx.offset = update_data.tellp(); // Get the current position in the update data file for the offset
    // Write the UpdateIdx to the index file
    update_idx.write((char*)&uidx, sizeof(UpdateIdx)); // Write the UpdateIdx structure to the index file
    // Write the differences for bids to the update data file
}

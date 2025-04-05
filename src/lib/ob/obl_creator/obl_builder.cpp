#include "obl_builder.hpp"
#include <fstream>
#include <iostream>
#include "../../config/config.hpp"
#include <string>
#include <algorithm>
#include <set>


using namespace std;


const size_t full_update_every_ms = 5 * 60 * 1000; // 5 minutes in milliseconds, for example, to trigger a full snapshot


ob::OBLB::OBLB(string symbol) : ob::OB(symbol), levelizer(symbol) {
    string obl_index_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "obl.idx";
    string obl_binary_path = config.get_path("data_path") + "um/depth/" + symbol + "/" + "obl.bin";
    obl_idx.open(obl_index_path, std::ios::binary);
    obl_data.open(obl_binary_path, std::ios::binary);
    last_full_ts = 0;
    bidsl.resize(levelizer.levels.size(), 0); // Initialize bidsl with zeros for each level
    asksl.resize(levelizer.levels.size(), 0); // Initialize asksl with zeros for each level
}

void ob::OBLB::apply_price_vol_snapshot(const ob::SnapshotIdx & sidx) {
    fill(bidsl.begin(), bidsl.end(), 0); // Reset bidsl to zero for each level
    fill(asksl.begin(), asksl.end(), 0); // Reset asksl to zero for each level

    for (size_t i = 0; i < bp.size(); i++) {
        if (bv[i] == 0) {
            bids.erase(bp[i]);
        } else {
            bids[bp[i]] = bv[i];
            if (levelizer.is_in_range(bp[i])) { // Only update bidsl if the price is in range
                bidsl[levelizer.get_level(bp[i])] += bv[i]; // Accumulate volume at the level
            }
        }
    }
    for (size_t i = 0; i < ap.size(); i++) {
        if (av[i] == 0) {
            asks.erase(ap[i]);
        } else {
            asks[ap[i]] = av[i];
            if (levelizer.is_in_range(ap[i])) { // Only update asksl if the price is in range
                asksl[levelizer.get_level(ap[i])] += av[i]; // Accumulate volume at the level
            }
        }
    }

    // write snapshot as full update to obl_idx and obl_data
    OBLRec oblr;
    oblr.t = sidx.t; // Timestamp of the snapshot
    oblr.full = true; // Mark this as a full snapshot
    oblr.offset = obl_data.tellp(); // Current position in the data file
    oblr.bid_size = levelizer.levels.size(); // Number of levels for bids
    oblr.ask_size = levelizer.levels.size(); // Number of levels for asks
    
    obl_idx.write((char*)&oblr, sizeof(OBLRec));
    obl_data.write((char*)bidsl.data(), sizeof(double) * levelizer.levels.size()); // Write the bidsl array to the data file
    obl_data.write((char*)asksl.data(), sizeof(double) * levelizer.levels.size()); // Write the asksl array to the data file
    last_full_ts = sidx.t; // Update the last full timestamp
}


void ob::OBLB::apply_price_vol_update(const ob::UpdateIdx & uidx) {
    double current_vol = 0;
    double delta;
    size_t l;
    bool is_in_range;
    set<size_t> bids_changed;
    set<size_t> asks_changed;

    for (size_t i = 0; i < bp.size(); i++) {
        is_in_range = levelizer.is_in_range(bp[i]); // Check if the price is within the range defined by PipLevelizer
        l = is_in_range ? levelizer.get_level(bp[i]) : 0; // Get the level for the price if in range
        current_vol = bids.contains(bp[i]) ? bids[bp[i]] : 0; // Get the current volume for this price
        delta = bv[i] - current_vol; // Calculate the difference between new volume and current volume
        if (is_in_range && delta != 0) {
            bidsl[l] += delta; // Update the bidsl for the level
            bids_changed.insert(l); // Track which levels have changed for bids
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
            asksl[l] += delta; // Update the asksl for the level
            asks_changed.insert(l); // Track which levels have changed for asks
        }

        if (av[i] == 0) {
            asks.erase(ap[i]);
        } else {
            asks[ap[i]] = av[i];
        }
    }

    // Write update to obl_idx and obl_data
    
    // only write update if its ts is greater than the last full timestamp
    if (false && uidx.t <= last_full_ts) {
        cout << "Skipping write for update with ts: " << uidx.t 
             << " as it is not greater than the last full timestamp: " 
             << last_full_ts << endl;
    }
    else{

        if (uidx.t - last_full_ts >= full_update_every_ms) {
            // write full snapshot as update
            // cout << "Writing full snapshot due to time threshold exceeded for ts: " << uidx.t << endl;
            OBLRec oblr;
            oblr.t = uidx.t; // Timestamp of the update
            oblr.full = true; // Mark this as a full snapshot
            oblr.offset = obl_data.tellp(); // Current position in the data file
            oblr.bid_size = levelizer.levels.size(); // Number of levels for bids
            oblr.ask_size = levelizer.levels.size(); // Number of levels for asks

            obl_idx.write((char*)&oblr, sizeof(OBLRec));
            obl_data.write((char*)bidsl.data(), sizeof(double) * levelizer.levels.size()); // Write the bidsl array to the data file
            obl_data.write((char*)asksl.data(), sizeof(double) * levelizer.levels.size()); // Write the asksl array to the data file
            last_full_ts = uidx.t; // Update the last full timestamp
        } else {
            // Write the differential update to the obl_data file
            OBLRec oblr;
            oblr.t = uidx.t; // Timestamp of the update
            oblr.full = false; // Mark this as a differential update
            oblr.offset = obl_data.tellp(); // Current position in the data file
            oblr.bid_size = bids_changed.size(); // Number of levels for bids that changed
            oblr.ask_size = asks_changed.size(); // Number of levels for asks that changed
     
            obl_idx.write((char*)&oblr, sizeof(OBLRec)); // Write the OBLRec structure to the index file
            for (const size_t& lev : bids_changed) {
                obl_data.write((char*)&lev, sizeof(size_t)); // Write the level (size_t) for the bid
                obl_data.write((char*)&bidsl[lev], sizeof(double)); // Write the volume (double) for the bid
            }
            for (const size_t& lev : asks_changed) {
                obl_data.write((char*)&lev, sizeof(size_t)); // Write the level (size_t) for the ask
                obl_data.write((char*)&asksl[lev], sizeof(double)); // Write the volume (double) for the ask
            }
        }
    }

}



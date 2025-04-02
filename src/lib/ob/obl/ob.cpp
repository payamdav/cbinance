#include "ob.hpp"
#include <iostream>
#include <string>
#include "../../utils/timer.hpp"
#include "../../utils/datetime_utils.hpp"
#include <algorithm> // For std::fill
#include <cstdlib>

double eps = 0.00000001;

obl::OB::OB(string symbol) {
    this->symbol = symbol;
    snapshot = new Snapshot(symbol);
    update = new Update(symbol);
    snapshot->open();
    update->open();
    PipLevelizer levelizer(symbol); // Create a levelizer for the symbol to initialize pip levels
    bids.resize(levelizer.levels.size(), 0); // Initialize bids vector with the number of levels
    asks.resize(levelizer.levels.size(), 0); // Initialize asks vector with the number of levels
    t = 0;
    u_id = 0;
    scount = snapshot->count();
    ucount = update->count();
    idx = 0; // Initialize the current index in the order book
    std::cout << "OB constructor called for: " << symbol << std::endl;
}

obl::OB::~OB() {
    // std::cout << "OB destructor called for: " << symbol << std::endl;
    delete snapshot;
    delete update;
}

void obl::OB::build(size_t from_ts, size_t to_ts) {
    // cout << "Building OB for symbol: " << symbol << endl;
    size_t ts_to_go = from_ts;
    size_t sid;
    size_t uid;
    utils::Timer timer(symbol + "_ob_timer");

    while (ts_to_go < to_ts) {
        if (!find_sid_uid(ts_to_go, sid, uid)) {
            // cout << "No snapshot with relevant update found for ts: " << ts_to_go << endl;
            return;
        }

        SnapshotIdxLevel sidx = snapshot->get_index(sid);
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
                ts_to_go = uidx.t;
                break;
            }
            apply_update(uidx);

            // if (uid % 100000 == 0) {
            //     timer.checkpoint();
            // }
        }

    }
    
}

bool obl::OB::find_sid_uid(size_t ts_to_go, size_t & sid, size_t & uid) {
    sid = snapshot->get_index_gte(ts_to_go);
    while(sid < scount) {
        SnapshotIdxLevel sidx = snapshot->get_index(sid);
        uid = update->get_index_relevant_to_snapshot(sidx);
        if (uid < ucount) return true;
        sid++;
    }
    return false;
}

void obl::OB::apply_snapshot(obl::SnapshotIdxLevel & sidx) {
    fill(bids.begin(), bids.end(), 0); // Clear bids vector
    fill(asks.begin(), asks.end(), 0); // Clear asks vector
    
    this->t = sidx.t;
    this->u_id = sidx.u_id;
    vector<double> b,a;
    snapshot->get_snapshot(sidx, b, a);
    // Apply price and volume to bids and asks for snapshot
    for (size_t i = 0; i < b.size(); i++) {
        bids[sidx.first_bid_level + i] = b[i]; // Set bids based on the snapshot data
    }
    for (size_t i = 0; i < a.size(); i++) {
        asks[sidx.first_ask_level + i] = a[i]; // Set asks based on the snapshot data
    }
    fb = sidx.first_bid_level; // Set first bid level
    lb = sidx.last_bid_level; // Set last bid level
    fa = sidx.first_ask_level; // Set first ask level
    la = sidx.last_ask_level; // Set last ask level

    // cout << "Snapshot applied. fb: " << fb << " lb: " << lb << " fa: " << fa << " la: " << la << endl;
    // for (size_t i = fb; i <= lb; i++) {
    //     cout << "Bid Level: " << i << ", Price: " << bids[i] << endl;
    // }
    // for (size_t i = fa; i <= la; i++) {
    //     cout << "Ask Level: " << i << ", Price: " << asks[i] << endl;
    // }
    // cout << " - - - - - - - - - - - - - " << endl;

    after_update();
    idx++;
}

void obl::OB::apply_update(obl::UpdateIdx & uidx) {
    this->t = uidx.t;
    this->u_id = uidx.u_id;
    unordered_map<size_t, double> b, a;
    update->get_update(uidx, b, a);
    // Apply price and volume to bids and asks for update
    for (const auto& pair : b) {
        bids[pair.first] += pair.second; // Update the bids based on the update data
        if (bids[pair.first] < eps && bids[pair.first] > -eps) {
            bids[pair.first] = 0; // Ensure bids are non-negative
        }
        else if (bids[pair.first] < -eps) {
            if (true) {
                cout << "Negative bid volume detected at level: " << pair.first << " - Volume: " << bids[pair.first] << endl;
                exit(1); // Exit if negative volume is detected
            }
        }
    }
    for (const auto& pair : a) {
        asks[pair.first] += pair.second; // Update the asks based on the update data
        if (asks[pair.first] < eps && asks[pair.first] > -eps) {
            asks[pair.first] = 0; // Ensure asks are non-negative
        }
        else if (asks[pair.first] < -eps) {
            if (true) {
                cout << "Negative ask volume detected at level: " << pair.first << " - Volume: " << asks[pair.first] << endl;
                exit(1); // Exit if negative volume is detected
            }
        }
    }
    // set fb, lb, fa, la
    for (size_t i = 0; i < bids.size(); i++) {
        if (bids[i] > eps) {
            fb = i;
            break;
        }
    }
    for (size_t i = bids.size() - 1; i > 0; i--) {
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
    for (size_t i = asks.size() - 1; i > 0; i--) {
        if (asks[i] > eps) {
            la = i;
            break;
        }
    }

    after_update();
    idx++;
}


void obl::OB::after_update() {
    // cout << "After update called" << endl;
    this->on_after_update(); // Call the virtual function for any additional processing
}


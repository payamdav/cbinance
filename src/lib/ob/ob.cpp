#include "ob.hpp"
#include <iostream>
#include <string>
#include "../utils/timer.hpp"
#include "../utils/datetime_utils.hpp"



OB::OB(string symbol) {
    this->symbol = symbol;
    snapshot = new Snapshot(symbol);
    update = new Update(symbol);
    snapshot->open();
    update->open();
    t = 0;
    u_id = 0;
    bp.reserve(10000);
    bv.reserve(10000);
    ap.reserve(10000);
    av.reserve(10000);
    scount = snapshot->count();
    ucount = update->count();
    idx = 0; // Initialize the current index in the order book
    std::cout << "OB constructor called for: " << symbol << std::endl;
}

OB::~OB() {
    // std::cout << "OB destructor called for: " << symbol << std::endl;
    delete snapshot;
    delete update;
}

void OB::build(size_t from_ts, size_t to_ts) {
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
                cout << "update gap detected. at uid: " << uid << " - ts: " << uidx.t << " - " << utils::get_utc_datetime_string(uidx.t) << endl;
                ts_to_go = uidx.t;
                break;
            }
            apply_update(uidx);

            if (uid % 100000 == 0) {
                timer.checkpoint();
            }
        }

    }
    
}

bool OB::find_sid_uid(size_t ts_to_go, size_t & sid, size_t & uid) {
    // cout << "Finding sid uid for ts: " << ts_to_go << " - " << utils::get_utc_datetime_string(ts_to_go) << endl;
    sid = snapshot->get_index_gte(ts_to_go);
    while(sid < scount) {
        SnapshotIdx sidx = snapshot->get_index(sid);
        if (sidx.t < ts_to_go) break;
        // cout << "Found snapshot at sid: " << sid << " - ts: " << sidx.t << " - " << utils::get_utc_datetime_string(sidx.t) << endl;
        uid = update->get_index_relevant_to_snapshot(sidx);
        if (uid < ucount) return true;
        sid++;
    }
    return false;
}

void OB::apply_snapshot(SnapshotIdx & sidx) {
    this->t = sidx.t;
    this->u_id = sidx.u_id;
    snapshot->get_snapshot(sidx, bp, bv, ap, av);
    this->apply_price_vol_snapshot(sidx); // Apply price and volume to bids and asks for snapshot
    after_update();
    idx++;
}

void OB::apply_update(UpdateIdx & uidx) {
    this->t = uidx.t;
    this->u_id = uidx.u_id;
    update->get_update(uidx, bp, bv, ap, av);
    this->apply_price_vol_update(uidx); // Apply price and volume to bids and asks for update
    after_update();
    idx++;
}

void OB::apply_price_vol_snapshot(const SnapshotIdx & sidx) {
    for (size_t i = 0; i < bp.size(); i++) {
        if (bv[i] == 0) {
            bids.erase(bp[i]);
        } else {
            bids[bp[i]] = bv[i];
        }
    }
    for (size_t i = 0; i < ap.size(); i++) {
        if (av[i] == 0) {
            asks.erase(ap[i]);
        } else {
            asks[ap[i]] = av[i];
        }
    }
}

void OB::apply_price_vol_update(const UpdateIdx & uidx) {
    for (size_t i = 0; i < bp.size(); i++) {
        if (bv[i] == 0) {
            bids.erase(bp[i]);
        } else {
            bids[bp[i]] = bv[i];
        }
    }
    for (size_t i = 0; i < ap.size(); i++) {
        if (av[i] == 0) {
            asks.erase(ap[i]);
        } else {
            asks[ap[i]] = av[i];
        }
    }
}

void OB::after_update() {
    // cout << "After update called" << endl;
    this->on_after_update(); // Call the virtual function for any additional processing
}

void OB::check() {
    double max_bids_price = 0;
    double min_bids_price = 1e9;
    double max_asks_price = 0;
    double min_asks_price = 1e9;
    double max_bids_volume = 0;
    double min_bids_volume = 1e9;
    double max_asks_volume = 0;
    double min_asks_volume = 1e9;
    size_t count_bids = 0;
    size_t count_asks = 0;
    double total_bids_volume = 0;
    double total_asks_volume = 0;
    bool is_bids_sorted = true;
    bool is_asks_sorted = true;
    size_t count_bids_price_small = 0;
    size_t count_asks_price_small = 0;
    double price_limit = 0.00000001; // Example price limit for small values
    double prev_price = -100;

    // Check bids
    for (const auto& bid : bids) {
        double price = bid.first;
        double volume = bid.second;

        count_bids++;
        total_bids_volume += volume;

        if (price > max_bids_price) {
            max_bids_price = price;
        }
        if (price < min_bids_price) {
            min_bids_price = price;
        }
        if (price < price_limit) {
            count_bids_price_small++;
        }

        if (volume > max_bids_volume) {
            max_bids_volume = volume;
        }
        if (volume < min_bids_volume) {
            min_bids_volume = volume;
        }

        if (price <= prev_price) {
            is_bids_sorted = false;
        }
        prev_price = price;
    }

    // Check asks
    prev_price = -100; // Reset previous price for asks
    for (const auto& ask : asks) {
        double price = ask.first;
        double volume = ask.second;

        count_asks++;
        total_asks_volume += volume;

        if (price > max_asks_price) {
            max_asks_price = price;
        }
        if (price < min_asks_price) {
            min_asks_price = price;
        }
        if (price < price_limit) {
            count_asks_price_small++;
        }

        if (volume > max_asks_volume) {
            max_asks_volume = volume;
        }
        if (volume < min_asks_volume) {
            min_asks_volume = volume;
        }

        if (price <= prev_price) {
            is_asks_sorted = false;
        }
        prev_price = price;
    }
    // Print results
    cout << "Bids: " << count_bids << " | Max Price: " << max_bids_price 
         << " | Min Price: " << min_bids_price 
         << " | Max Volume: " << max_bids_volume 
         << " | Min Volume: " << min_bids_volume 
         << " | Total Volume: " << total_bids_volume 
         << " | Sorted: " << (is_bids_sorted ? "Yes" : "No") 
         << " | Count Small Prices: " << count_bids_price_small 
         << endl;
    cout << "Asks: " << count_asks << " | Max Price: " << max_asks_price
         << " | Min Price: " << min_asks_price 
         << " | Max Volume: " << max_asks_volume 
         << " | Min Volume: " << min_asks_volume 
         << " | Total Volume: " << total_asks_volume 
         << " | Sorted: " << (is_asks_sorted ? "Yes" : "No") 
         << " | Count Small Prices: " << count_asks_price_small
         << endl;

}

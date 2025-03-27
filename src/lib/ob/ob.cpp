#include "ob.hpp"
#include <iostream>
#include <string>



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
    std::cout << "OB constructor called for: " << symbol << std::endl;
}

OB::~OB() {
    std::cout << "OB destructor called for: " << symbol << std::endl;
    delete snapshot;
    delete update;
}

void OB::build(size_t from_ts, size_t to_ts) {
    cout << "Building OB for symbol: " << symbol << endl;
    size_t ts_to_go = from_ts;
    size_t sid;
    size_t uid;

    while (ts_to_go < to_ts) {
        if (!find_sid_uid(ts_to_go, sid, uid)) {
            cout << "No snapshot with relevant update found for ts: " << ts_to_go << endl;
            return;
        }
        SnapshotIdx sidx = snapshot->get_index(sid);
        apply_snapshot(sidx);
        UpdateIdx uidx = update->get_index(uid);
        apply_update(uidx);
        uid++;

        for (; uid < ucount; uid++) {
            uidx = update->get_index(uid);
            if (uidx.pu_id != u_id) {
                cout << "update gap detected. at uid: " << uid << endl;
                ts_to_go = uidx.t;
                break;
            }
            apply_update(uidx);
            if (uid % 1000 == 0) {
                cout << "Processed update: " << uid << endl;
            }
        }
    }
    
}

void OB::clear_temp_vectores() {
    bp.clear();
    bv.clear();
    ap.clear();
    av.clear();
}

bool OB::find_sid_uid(size_t ts_to_go, size_t & sid, size_t & uid) {
    cout << "Finding sid uid for ts: " << ts_to_go << endl;
    sid = snapshot->get_index_gte(ts_to_go);
    while(sid < scount) {
        SnapshotIdx sidx = snapshot->get_index(sid);
        if (sidx.t < ts_to_go) break;
        uid = update->get_index_relevant_to_snapshot(sidx);
        if (uid < ucount) return true;
        sid++;
    }
    return false;
}

void OB::apply_snapshot(SnapshotIdx & sidx) {
    this->t = sidx.t;
    this->u_id = sidx.u_id;
    this->clear_temp_vectores();
    snapshot->get_snapshot(sidx, bp, bv, ap, av);
    this->apply_price_vol();
    after_update();
}

void OB::apply_update(UpdateIdx & uidx) {
    this->t = uidx.t;
    this->u_id = uidx.u_id;
    this->clear_temp_vectores();
    update->get_update(uidx, bp, bv, ap, av);
    this->apply_price_vol();
    after_update();
}

void OB::apply_price_vol() {
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
}
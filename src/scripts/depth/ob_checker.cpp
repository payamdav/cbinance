#include <iostream>
#include "../../lib/ob/snapshot.hpp"
#include "../../lib/ob/update.hpp"
#include <string>
#include <vector>
#include "../../lib/config/config.hpp"


using namespace std;


void snapshot_test(int argc, char *argv[]) {
    string symbol = "adausdt";
    size_t idx = 0;

    vector<double> bp;
    vector<double> bv;
    vector<double> ap;
    vector<double> av;

    bp.reserve(10000);
    bv.reserve(10000);
    ap.reserve(10000);
    av.reserve(10000);

    if (argc == 3) {
        symbol = argv[1];
        idx = stoi(argv[2]);
    }

    ob::Snapshot snapshot(symbol);
    snapshot.open();
    cout << "Snapshot count: " << snapshot.count() << endl;
    cout << "Snapshot index: " << idx << endl;
    ob::SnapshotIdx snapshot_idx = snapshot.get_index(idx);
    cout << "Snapshot index: " << snapshot_idx << endl;

    // print horizontal line
    cout << string(80, '-') << endl;

    snapshot.get_snapshot(snapshot_idx, bp, bv, ap, av);
    for (size_t i = 0; i < snapshot_idx.bid_size; i++) {
        cout << bp[i] << " " << bv[i] << endl;
    }
    cout << string(80, '-') << endl;
    for (size_t i = 0; i < snapshot_idx.ask_size; i++) {
        cout << ap[i] << " " << av[i] << endl;
    }
    snapshot.close();

}

void update_test(int argc, char *argv[]) {
    string symbol = "adausdt";
    size_t idx = 0;

    vector<double> bp;
    vector<double> bv;
    vector<double> ap;
    vector<double> av;

    bp.reserve(10000);
    bv.reserve(10000);
    ap.reserve(10000);
    av.reserve(10000);

    if (argc == 3) {
        symbol = argv[1];
        idx = stoi(argv[2]);
    }

    ob::Update update(symbol);
    update.open();
    cout << "Update count: " << update.count() << endl;
    cout << "Update index: " << idx << endl;
    ob::UpdateIdx updateidx = update.get_index(idx);
    cout << "Update index: " << updateidx << endl;

    // print horizontal line
    cout << string(80, '-') << endl;

    update.get_update(updateidx, bp, bv, ap, av);
    for (size_t i = 0; i < updateidx.bid_size; i++) {
        cout << bp[i] << " " << bv[i] << endl;
    }
    cout << string(80, '-') << endl;
    for (size_t i = 0; i < updateidx.ask_size; i++) {
        cout << ap[i] << " " << av[i] << endl;
    }
    update.close();

}

void check_snapshots(int argc, char *argv[]) {
    auto symbols = config.get_csv_strings("symbols_list");
    for (auto symbol : symbols) {
        ob::Snapshot snapshot(symbol);
        snapshot.open();
        cout << "Snapshot count: " << snapshot.count() << endl;
        cout << "Is sorted by t: " << snapshot.is_sorted_by_t() << endl;
        cout << "Is sorted by u_id: " << snapshot.is_sorted_by_u_id() << endl;
        snapshot.close();
        cout << string(80, '-') << endl;
    }
}

void check_updates(int argc, char *argv[]) {
    auto symbols = config.get_csv_strings("symbols_list");
    for (auto symbol : symbols) {
        ob::Update update(symbol);
        update.open();
        cout << "Update count: " << update.count() << endl;
        // cout << "Is sorted by t: " << update.is_sorted_by_t() << endl;
        // cout << "Is sorted by u_id: " << update.is_sorted_by_u_id() << endl;
        cout << "Is p_u aligned: " << update.is_pu_aligned() << endl;
        // cout << "Is index size correct: " << update.is_index_size_correct() << endl;
        // cout << "Is offset correct: " << update.is_offset_correct() << endl;
        update.close();
        cout << string(80, '-') << endl;
    }
}




int main(int argc, char *argv[]) {
    // snapshot_test(argc, argv);
    // check_snapshots(argc, argv);
    // update_test(argc, argv);
    check_updates(argc, argv);

    return 0;
}

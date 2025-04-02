#include "snapshot_converter.hpp"
#include <string>
#include <iostream>
#include "../../config/config.hpp"
#include "../../utils/file_utils.hpp"
#include "../../utils/string_utils.hpp"
#include "../../ta/pip_levelizer/pip_levelizer.hpp" // Include PipLevelizer if needed for conversion logic
#include <fstream>
#include <vector>
#include <algorithm> // For std::fill





ob::SnapshotConverter::SnapshotConverter(string symbol) : snapshot(symbol), levelizer(symbol) {
    symbol = utils::toLowerCase(symbol); // Ensure symbol is lowercase
    this->symbol = symbol;
    snapshot.open(); // Open the snapshot files to read
    snapshot_idx.open(config.get_path("data_path") + "um/depth/" + symbol + "/" + "snapshot_level.idx", std::ios::binary | std::ios::app);
    snapshot_data.open(config.get_path("data_path") + "um/depth/" + symbol + "/" + "snapshot_level.bin", std::ios::binary | std::ios::app);
    a.resize(levelizer.levels.size()); // Initialize ask vector with zeros for each level
    b.resize(levelizer.levels.size()); // Initialize bid vector with zeros for each level
}

void ob::SnapshotConverter::convert_to_level_snapshot() {
    size_t count = snapshot.count();
    if (count == 0) {
        std::cerr << "No snapshots to convert for symbol: " << symbol << std::endl;
        return;
    }

    for (size_t i = 0; i < count; i++) {
        vector<double> bp, bv, ap, av;
        SnapshotIdx snapshotidx = snapshot.get_index(i);
        snapshot.get_snapshot(snapshotidx, bp, bv, ap, av);

        // Reset the level vectors
        fill(b.begin(), b.end(), 0);
        fill(a.begin(), a.end(), 0);

        // Aggregate volumes at each level
        for (size_t j = 0; j < bp.size(); j++) {
            if (levelizer.is_in_range(bp[j])) {
                b[levelizer.get_level(bp[j])] += bv[j];
            }
        }

        for (size_t j = 0; j < ap.size(); j++) {
            if (levelizer.is_in_range(ap[j])) {
                a[levelizer.get_level(ap[j])] += av[j];
            }
        }

        size_t b1, b2, a1, a2;
        b1 = b2 = a1 = a2 = 0; // Initialize first and last levels for bids and asks
        for (size_t j = 0; j < b.size(); j++) {
            if (b[j] > 0) {
                b1 = j; // First level with non-zero volume
                break; // Break after finding the first level
            }
        }

        for (size_t j = b.size() - 1; j >= 0; j--) {
            if (b[j] > 0) {
                b2 = j; // Last level with non-zero volume
                break; // Break after finding the last level
            }
        }

        for (size_t j = 0; j < a.size(); j++) {
            if (a[j] > 0) {
                a1 = j; // First level with non-zero volume
                break; // Break after finding the first level
            }
        }

        for (size_t j = a.size() - 1; j >= 0; j--) {
            if (a[j] > 0) {
                a2 = j; // Last level with non-zero volume
                break; // Break after finding the last level
            }
        }

        SnapshotIdxLevel sidx; // Copy the current snapshot index
        sidx.t = snapshotidx.t; // Time of the snapshot
        sidx.u_id = snapshotidx.u_id; // Unique ID of the snapshot
        sidx.first_bid_level = b1; // First level with non-zero volume for bids
        sidx.last_bid_level = b2; // Last level with non-zero volume for bids
        sidx.first_ask_level = a1; // First level with non-zero volume for asks
        sidx.last_ask_level = a2; // Last level with non-zero volume for asks
        sidx.size = (b2 - b1 + 1 + a2 - a1 + 1) * sizeof(double); // Calculate the size of the snapshot based on the levels with non-zero volumes
        sidx.offset = snapshot_data.tellp(); // Get the current position in the snapshot data file for the offset

        // Write the index and data to the new files
        // Write the SnapshotIdxLevel to the index file
        snapshot_idx.write(reinterpret_cast<const char*>(&sidx), sizeof(SnapshotIdxLevel)); // Write the index to the index file
        // Write the bid levels to the snapshot data file
        snapshot_data.write(reinterpret_cast<const char*>(&b[b1]), (b2 - b1 + 1) * sizeof(double)); // Write the bid levels from b1 to b2
        // Write the ask levels to the snapshot data file
        snapshot_data.write(reinterpret_cast<const char*>(&a[a1]), (a2 - a1 + 1) * sizeof(double)); // Write the ask levels from a1 to a2
        // Note: The above writes the levels from b1 to b2 for bids and a1 to a2 for asks
        // This ensures that only the levels with non-zero volumes are written to the snapshot data file    
    }
}




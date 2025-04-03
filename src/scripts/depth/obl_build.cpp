#include <iostream>
#include <string>
#include <vector>
#include "../../lib/config/config.hpp"
#include "../../lib/ob/obl_creator/obl_creator.hpp"
#include "../../lib/ob/obl_creator/obl_builder.hpp" // Include the OBLB class for building the order book
#include "../../lib/utils/timer.hpp" // Include the timer utility for performance measurement
#include "../../lib/ob/obl_creator/snapshot_converter.hpp"


using namespace std;


void obl_creator(string symbol) {
    utils::Timer timer(symbol + "_obl_creator_timer"); // Timer for performance measuremen
    ob::OBLC obl(symbol);
    obl.build();
    obl.update_idx.close(); // Close the index file after building
    obl.update_data.close(); // Close the binary file after building
    timer.checkpoint(); // Checkpoint after building the order book
}

void obl_snapshot_converter(string symbol) {
    utils::Timer timer(symbol + "_snapshot_converter_timer"); // Timer for performance measurement
    ob::SnapshotConverter snapshot_converter(symbol); // Create an instance of SnapshotConverter for the given symbol
    snapshot_converter.convert_to_level_snapshot(); // Convert the snapshots to level snapshots
    snapshot_converter.snapshot_idx.close(); // Close the index file after conversion
    snapshot_converter.snapshot_data.close(); // Close the binary file after conversion
    // The conversion process is complete, and the files are closed
    timer.checkpoint(); // Checkpoint after building the order book
}

void obl_build(string symbol) {
    utils::Timer timer(symbol + "_obl_builder_timer"); // Timer for performance measurement
    ob::OBLB obl(symbol); // Create an instance of OBLB for the given symbol
    obl.build(); // Build the order book
    obl.obl_idx.close(); // Close the index file after building
    obl.obl_data.close(); // Close the binary file after building
    timer.checkpoint(); // Checkpoint after building the order book
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " build or updates or snapshots" << std::endl;
        return 1;
    }

    vector<string> symbols = config.get_csv_strings("symbols_list");
    if (std::string(argv[1]) == "updates") {
        for (const auto& symbol : symbols) {
            obl_creator(symbol);
        }
    } else if (std::string(argv[1]) == "snapshots") {
        for (const auto& symbol : symbols) {
            obl_snapshot_converter(symbol);
        }
    } else if (std::string(argv[1]) == "build") {
        for (const auto& symbol : symbols) {
            obl_build(symbol);
        }
    } else {
        std::cerr << "Invalid argument. Use 'build' or 'updates' or 'snapshots'." << std::endl;
        return 1;
    }

    return 0;
}
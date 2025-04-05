#include <iostream>
#include <string>
#include <vector>
#include "../../lib/config/config.hpp"
// #include "../../lib/ob/obl/snapshot.hpp"
// #include "../../lib/ob/obl/update.hpp"
#include "../../lib/ob/ob.hpp"
#include "../../lib/ob/obl/ob.hpp"
#include "../../lib/ob/obl/obl.hpp"
#include "../../lib/ob/obl_creator/obl_builder.hpp"
#include "../../lib/utils/timer.hpp" // Include the timer utility for performance measurement
#include <cstdlib> // For exit function
#include <cmath>

using namespace std;



void ob_test_1(int argc, char *argv[]) {
    // auto symbols = config.get_csv_strings("symbols_list");
    // OB ob("btcusdt");
    obl::OB ob("dogeusdt");
    // ob.build(1742319330205);
    ob.build();

    // for (size_t i = ob.lb - 20; i <= ob.lb+1; i++) {
    //     cout << "Bid Level: " << i << ", Price: " << ob.bids[i] << endl;
    // }
    // for (size_t i = ob.fa-1; i <= ob.fa + 20; i++) {
    //     cout << "Ask Level: " << i << ", Price: " << ob.asks[i] << endl;
    // }
}

void ob_obl_check(string symbol) {
    // ob::OB ob(symbol);
    // ob.build();

    // cout << "-------------------------" << endl;

    obl::OB obl(symbol);
    obl.build();
    cout << "-------------------------" << endl;

}

class OBLChecker : public obl::OB {
    public:
        obl::OBL * obl_instance;
        OBLChecker(string symbol) : obl::OB(symbol) {
            // Initialize the OBL instance for this checker
            obl_instance = new obl::OBL(symbol);
            obl_instance->init(); // Initialize the OBL instance, this will set up the order book
        }

        ~OBLChecker() {
            if (obl_instance) {
                delete obl_instance; // Clean up the OBL instance
            }
        }

        void on_after_update() override {
            if (this->t != obl_instance->t) {
                // If the timestamps do not match, print a warning
                std::cerr << "Timestamp mismatch: OB t = " << this->t << ", OBL t = " << obl_instance->t << " - at index: " << obl_instance->idx << std::endl;
                // exit(1); // Exit the program to avoid further inconsistencies
                obl_instance->next(); // Move to the next record in the OBL instance to try and sync
                cout << "Attempting to sync to next OBL record with ts: " << obl_instance->t << " at index: " << obl_instance->idx << endl;
            }

            for (size_t i = 0; i < bids.size(); i++) {
                if (abs(bids[i] - obl_instance->bids[i]) > obl::eps) {
                    // If the bid prices do not match, print a warning
                    std::cerr << "Bid price mismatch at level " << i << ": OB bids[" << i << "] = " << bids[i] << ", OBL bids[" << i << "] = " << obl_instance->bids[i] << " at index: " << obl_instance->idx << endl;
                    exit(1); // Exit to avoid further inconsistencies
                }
                if (abs(asks[i] - obl_instance->asks[i]) > obl::eps) {
                    // If the ask prices do not match, print a warning
                    std::cerr << "Ask price mismatch at level " << i << ": OB asks[" << i << "] = " << asks[i] << ", OBL asks[" << i << "] = " << obl_instance->asks[i] << " at index: " << obl_instance->idx << endl;
                    exit(1); // Exit to avoid further inconsistencies
                }

            }

            if (!obl_instance->ended()) {
                // If the OBL instance has not ended, move to the next record
                obl_instance->next(); // Move to the next record in the OBL instance
            } else {
                cout << "Finished processing all OBL records." << endl;
            }
        }

};



void obl_build(string symbol) {
    utils::Timer timer(symbol + "_obl_builder_timer"); // Timer for performance measurement
    ob::OBLB obl(symbol); // Create an instance of OBLB for the given symbol
    obl.build(); // Build the order book
    obl.obl_idx.close(); // Close the index file after building
    obl.obl_data.close(); // Close the binary file after building
    timer.checkpoint(); // Checkpoint after building the order book
}


// class MyOB : public OB {
//     public:
//         MyOB(string symbol) : OB(symbol) {}
//         size_t count_negative_spreads = 0; // Count of negative spreads
//         double max_spread = 0; // Maximum spread observed
//         double min_spread = 1e9; // Minimum spread observed
//         double max_spread_percent = 0; // Maximum spread percentage observed

//         void on_after_update() override {
//             double best_bid = bids.rbegin()->first; // Get the best bid price
//             double best_ask = asks.begin()->first; // Get the best ask price
//             double spread = best_ask - best_bid; // Calculate the spread
//             if (spread < 0) {
//                 count_negative_spreads++; // Increment count if spread is negative
//             }
//             else {
//                 // Update max and min spreads
//                 if (spread > max_spread) {
//                     max_spread = spread; // Update maximum spread
//                 }
//                 if (spread < min_spread) {
//                     min_spread = spread; // Update minimum spread
//                 }
//                 // Calculate spread percentage
//                 double spread_percent = (spread / best_bid) * 100;
//                 if (spread_percent > max_spread_percent) {
//                     max_spread_percent = spread_percent; // Update maximum spread percentage
//                 }
//             }
//         }

//         void print_spread_statistics() {
//             cout << "Spread Statistics for " << symbol << ":" << endl;
//             cout << "Count of Negative Spreads: " << count_negative_spreads << endl;
//             cout << "Maximum Spread: " << max_spread << endl;
//             cout << "Minimum Spread: " << min_spread << endl;
//             cout << "Maximum Spread Percentage: " << max_spread_percent << "%" << endl;
//         }
// };

// void ob_test_2(int argc, char *argv[]) {
//     // auto symbols = config.get_csv_strings("symbols_list");
//     // MyOB ob("btcusdt");
//     MyOB ob("trumpusdt");
//     // MyOB ob("adausdt");
//     // ob.build(1742319330205);
//     ob.build();
//     ob.check(); // This will check the order book for any inconsistencies or issues
//     // After building the order book, print the spread statistics
//     ob.print_spread_statistics(); // Print the spread statistics
// }

// void test_min_max_price(int argc, char *argv[]) {
//     auto symbols = config.get_csv_strings("symbols_list");
//     for (const auto& symbol : symbols) {
//         // Create an instance of OBMinMaxPrice for each symbol
//         OBMinMaxPrice ob(symbol);
//         // Build the order book for the symbol
//         ob.build();
//         // Check for any inconsistencies or issues in the order book
//         ob.check();
//         // Save the min and max prices to a binary file
//         ob.save_min_max_price();
//     }
// }

// void test_min_max_price_read(int argc, char *argv[]) {
//     auto symbols = config.get_csv_strings("symbols_list");
//     double min_price, max_price; // Variables to hold min and max prices
//     for (const auto& symbol : symbols) {
//         bool res = read_min_max_price(symbol, min_price, max_price); // Read the min and max prices from the binary file
//         if (res) {
//             // If reading was successful, print the min and max prices
//             cout << "Symbol: " << symbol << ", Min Price: " << min_price << ", Max Price: " << max_price << endl;
//         } else {
//             // If there was an error reading the prices, print an error message
//             cout << "Failed to read min/max prices for symbol: " << symbol << endl;
//         }
//     }
// }

int main(int argc, char *argv[]) {
    // vector<string> symbols = config.get_csv_strings("symbols_list");
    // for (const auto& symbol : symbols) {
    //     OBLChecker obl_checker(symbol); // Create an instance of the OBLChecker for a specific symbol
    //     obl_checker.build(); // Build the order book using the OBLChecker
    // }

    // obl_build("vineusdt"); // Build the order book for a specific symbol, e.g., "vineusdt"




    OBLChecker obl_checker("vineusdt"); // Create an instance of the OBLChecker for a specific symbol
    obl_checker.build(); // Build the order book using the OBLChecker
    return 0;
};

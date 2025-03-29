#include "../../lib/trade/trade.hpp"
#include "../../lib/config/config.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;


int main(int argc, char *argv[]) {
    vector<string> symbols = config.get_csv_strings("symbols_list");
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << "year month day [count following days]" << endl;
        return 1;
    }
    else if (argc == 4) {
        int year = atoi(argv[1]);
        int month = atoi(argv[2]);
        int day = atoi(argv[3]);
        for (string symbol : symbols) {
            Trades trades(symbol);
            trades.import_from_csv(year, month, day);
        }
    } else if (argc == 5) {
        int year = atoi(argv[1]);
        int month = atoi(argv[2]);
        int day = atoi(argv[3]);
        int count = atoi(argv[4]);
        for (string symbol : symbols) {
            Trades trades(symbol);
            for (int i = 0; i < count; i++) {
                trades.import_from_csv(year, month, day + i);
            }
        }
    }
    return 0;
}

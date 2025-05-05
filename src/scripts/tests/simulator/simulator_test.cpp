#include <string>
#include <iostream>
#include "../../../lib/utils/datetime_utils.hpp"
#include "../../../lib/config/config.hpp"
#include "../../../lib/simulator/simulator.hpp"
#include "../../../lib/trade/trade.hpp"
#include "../../../lib/utils/timer.hpp"
#include "../../../lib/ta/pip_levelizer/pip_levelizer.hpp"
#include "../../../lib/utils/datetime_utils.hpp"
#include "../../../lib/market/market.hpp"


using namespace std;


void test1(string symbol) {
    // size_t ts1 = utils::get_timestamp("2025-03-14 00:00:00");
    // size_t ts2 = utils::get_timestamp("2025-03-15 00:00:00");
    Simulator *sim = new Simulator(symbol);
    sim->run();
}

class SimulatorTest : public Simulator {
    public:
        size_t count = 0;
        utils::Timer timer;
        PipLevelizer levelizer;
        size_t last_level = 0;
        size_t count_level_change = 0;
        size_t first_trade_ts = 0;
        size_t last_trade_ts = 0;
        LMarket market;
        

        SimulatorTest(string symbol) : Simulator(symbol), levelizer(symbol), market() {
        }

        void pre() override {
            cout << "Starting simulation for symbol: " << this->symbol << endl;
            // Custom setup logic can be added here
        }

        void new_trade(Trade &trade) override {
            // Custom logic for handling new trades can be added here
            count++;
            size_t current_level = levelizer(trade.p);
            if (current_level != last_level) {
                count_level_change++;
                last_level = current_level;
                auto order = market.market_order(OrderDirection::LONG);
                order.sl = current_level - 2;
                order.tp = current_level + 2;
            }
            market.push(trade.t, current_level);
            if (first_trade_ts == 0) {
                first_trade_ts = trade.t;
            }
            last_trade_ts = trade.t;
        }

        void post() override {
            timer.checkpoint("post");
            cout << "Simulation completed for symbol: " << this->symbol << " with " << count << " trades processed." << endl;
            cout << "Level changes detected: " << count_level_change << endl;
            cout << "First trade timestamp: " << utils::get_utc_datetime_string(first_trade_ts) << endl;
            cout << "Last trade timestamp: " << utils::get_utc_datetime_string(last_trade_ts) << endl;
            auto report = market.report();
            report.print();
        }
};

void test2(string symbol) {
    SimulatorTest *sim = new SimulatorTest(symbol);
    sim->start_ts = utils::get_timestamp("2025-03-14 00:00:00");
    sim->end_ts = utils::get_timestamp("2025-03-14 00:30:00");
    sim->run();
}

int main() {
    string symbol = "adausdt";
    cout << "Running simulator test for symbol: " << symbol << endl;
    test2(symbol);
    return 0;
}

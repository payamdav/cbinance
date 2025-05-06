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
        LMarket market_long;
        LMarket market_short;
        PipLevelizer levelizer;

        utils::Timer timer;
        size_t count_trades = 0;
        size_t count_levels = 0;
        size_t last_level = 0;
        

        SimulatorTest(string symbol) : Simulator(symbol), levelizer(symbol), market_long(symbol + "_long"), market_short(symbol + "_short") {
            market_long.set_commision(10);
            market_short.set_commision(10);
        }

        void pre() override {
            cout << "Starting simulation for symbol: " << this->symbol << endl;
            // Custom setup logic can be added here
        }

        void new_trade(Trade &trade) override {
            // Custom logic for handling new trades can be added here
            count_trades++;
            size_t current_level = levelizer(trade.p);
            if (current_level != last_level) {
                count_levels++;
                market_long.push(trade.t, current_level);
                market_short.push(trade.t, current_level);

                auto order = market_long.market_order(OrderDirection::LONG);
                order->sl = current_level - 1;
                order->tp = current_level + 60;

                auto order2 = market_short.market_order(OrderDirection::SHORT);
                order2->sl = current_level + 1;
                order2->tp = current_level - 60;

                last_level = current_level;
            }
        }

        void post() override {
            timer.checkpoint("post");
            cout << "Simulation completed for symbol: " << this->symbol << " with " << count_trades << " trades processed and " << count_levels << "Levels changed." << endl;
            auto report_long = market_long.report();
            auto report_short = market_short.report();
            cout << report_long << endl;
            cout << "----------------------------------------" << endl;
            cout << report_short << endl;
        }
};

void test2(string symbol) {
    SimulatorTest *sim = new SimulatorTest(symbol);
    sim->start_ts = utils::get_timestamp("2025-03-14 00:00:00");
    sim->end_ts = utils::get_timestamp("2025-03-30 00:00:00");
    sim->run();
}

int main() {
    string symbol = "adausdt";
    cout << "Running simulator test for symbol: " << symbol << endl;
    test2(symbol);
    return 0;
}

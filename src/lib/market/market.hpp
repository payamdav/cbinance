#pragma once
#include <string>
#include <list>

using namespace std;


enum class ExitStatus {
    UnCompleted,
    SL,
    TP,
    Cancel
};

enum class OrderDirection {
    LONG,
    SHORT
};


class LOrder {
    public:
        size_t id = 0;
        size_t request_ts = 0;
        size_t entry_ts = 0;
        size_t exit_ts = 0;
        ExitStatus exit_status = ExitStatus::UnCompleted;
        OrderDirection direction = OrderDirection::LONG;

        size_t activate_price_upper = 0; // Price to activate the order (for limit orders)
        size_t activate_price_lower = 0; // Price to activate the order (for limit orders)
        size_t cancel_price_upper = 0; // Price to cancel the order (for limit orders)
        size_t cancel_price_lower = 0; // Price to cancel the order (for limit orders)
        size_t cancel_duration = 0; // Duration to cancel the order (for limit orders)

        size_t entry_price = 0;
        size_t exit_price = 0;
        size_t sl = 0; // Stop Loss
        size_t tp = 0; // Take Profit
        size_t profit = 0; // Profit in pips
        size_t duration = 0; // Duration in seconds

        void finalize();
        void finalize(size_t exit_ts, size_t exit_price, ExitStatus exit_status);

};

struct LMarketReport {
    size_t total_orders = 0;
    size_t completed_orders = 0;
    size_t active_orders = 0;
    size_t pending_orders = 0;
    size_t total_profit = 0; // Total profit in pips
    size_t total_duration = 0; // Total duration in seconds

    void print() const;
};


class LMarket {
    private:
        void handle_pending_orders();
        void handle_active_orders();
    public:
        string name;
        size_t last_order_id = 0;
        size_t last_ts = 0; // Last timestamp of the market
        size_t last_price = 0.0; // Last price of the market
        list<LOrder> pending_orders;
        list<LOrder> active_orders;
        list<LOrder> completed_orders;

        LMarket(const string &name = "Default Market");
        void push(size_t ts, size_t price);
        LOrder & market_order(OrderDirection direction);
        LOrder & pending_order(OrderDirection direction);
        void cancel_order(size_t id);
        LMarketReport report() const;



};

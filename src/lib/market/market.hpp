#pragma once
#include <string>
#include <list>
#include <iostream>


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
        long profit = 0; // Profit in pips
        long net_profit = 0; // Net profit in pips
        size_t duration = 0; // Duration in seconds

        long commision = 0; // Commission in pips

        void finalize();
        void finalize(size_t exit_ts, size_t exit_price, ExitStatus exit_status);

};

ostream & operator<<(ostream &os, const LOrder &order);

struct LMarketReport {
    size_t total_orders = 0;
    size_t completed_orders = 0;
    size_t active_orders = 0;
    size_t pending_orders = 0;
    size_t success = 0;
    size_t failed = 0;
    size_t sl = 0; // Number of orders closed by Stop Loss
    size_t tp = 0; // Number of orders closed by Take Profit
    size_t cancel = 0; // Number of orders canceled
    double win_rate = 0; // Win rate in percentage
    double avg_duration = 0; // Average duration in seconds
    double avg_duration_success = 0; // Average duration of successful orders in seconds
    double avg_duration_failed = 0; // Average duration of failed orders in seconds
    long total_profit = 0; // Total profit in pips
    long total_net_profit = 0; // Total net profit in pips
};

ostream & operator<<(ostream &os, const LMarketReport &report);

class LMarket {
    private:
        void handle_pending_orders();
        void handle_active_orders();
    public:
        string name;
        size_t last_order_id = 0;
        size_t last_ts = 0; // Last timestamp of the market
        size_t last_price = 0; // Last price of the market
        long commision = 0; // Commission in pips
        list<LOrder> pending_orders;
        list<LOrder> active_orders;
        list<LOrder> completed_orders;

        LMarket(const string &name = "Default Market");
        void set_commision(long commision);
        void push(size_t ts, size_t price);
        LOrder * market_order(OrderDirection direction);
        LOrder * pending_order(OrderDirection direction);
        void cancel_order(size_t id);
        LMarketReport report() const;



};

#include "market.hpp"
#include <iostream>


using namespace std;

void LOrder::finalize() {
    duration = exit_ts - entry_ts;
    profit = (exit_price - entry_price) * (direction == OrderDirection::LONG ? 1 : -1);
}

void LOrder::finalize(size_t exit_ts, size_t exit_price, ExitStatus exit_status) {
    this->exit_ts = exit_ts;
    this->exit_price = exit_price;
    this->exit_status = exit_status;
    finalize();
}

LMarket::LMarket(const string &name) : name(name) {
    cout << "Market created: " << name << endl;
}

void LMarket::push(size_t ts, size_t price) {
    last_ts = ts;
    last_price = price;
    handle_pending_orders();
    handle_active_orders();
}

void LMarket::handle_pending_orders() {
    for (auto it = pending_orders.begin(); it != pending_orders.end();) {
        if (it->cancel_duration > 0 && (last_ts - it->request_ts) >= it->cancel_duration) {
            it = pending_orders.erase(it);
        }
        else if (it->cancel_price_lower > 0 && last_price <= it->cancel_price_lower) {
            it = pending_orders.erase(it);
        }
        else if (it->cancel_price_upper > 0 && last_price >= it->cancel_price_upper) {
            it = pending_orders.erase(it);
        }
        else if (it->activate_price_lower > 0 && last_price <= it->activate_price_lower) {
            it->entry_ts = last_ts;
            it->entry_price = last_price;
            auto next_it = std::next(it);
            active_orders.splice(active_orders.end(), pending_orders, it);
            it = next_it;
        } 
        else if (it->activate_price_upper > 0 && last_price >= it->activate_price_upper) {
            it->entry_ts = last_ts;
            it->entry_price = last_price;
            auto next_it = std::next(it);
            active_orders.splice(active_orders.end(), pending_orders, it);
            it = next_it;
        } 
        else {
            ++it;
        }
    }
}

void LMarket::handle_active_orders() {
    for (auto it = active_orders.begin(); it != active_orders.end();) {
        if (it->direction == OrderDirection::LONG && it->sl > 0 && last_price <= it->sl) {
            it->finalize(last_ts, last_price, ExitStatus::SL);
            auto next_it = std::next(it);
            completed_orders.splice(completed_orders.end(), active_orders, it);
            it = next_it;
        }
        else if (it->direction == OrderDirection::SHORT && it->sl > 0 && last_price >= it->sl) {
            it->finalize(last_ts, last_price, ExitStatus::SL);
            auto next_it = std::next(it);
            completed_orders.splice(completed_orders.end(), active_orders, it);
            it = next_it;
        }
        else if (it->tp > 0 && ((it->direction == OrderDirection::LONG && last_price >= it->tp) ||
                               (it->direction == OrderDirection::SHORT && last_price <= it->tp))) {
            it->finalize(last_ts, last_price, ExitStatus::TP);
            auto next_it = std::next(it);
            completed_orders.splice(completed_orders.end(), active_orders, it);
            it = next_it;
        }
        else {
            ++it;
        }

    }
}


LOrder & LMarket::market_order(OrderDirection direction) {
    LOrder &order = active_orders.emplace_back();
    order.id = ++last_order_id;
    order.direction = direction;
    order.request_ts = last_ts;
    order.entry_ts = last_ts;
    order.entry_price = last_price;
    return order;
}

LOrder & LMarket::pending_order(OrderDirection direction) {
    LOrder &order = pending_orders.emplace_back();
    order.id = ++last_order_id;
    order.direction = direction;
    order.request_ts = last_ts;
    return order;
}

void LMarket::cancel_order(size_t id) {
    for (auto it = pending_orders.begin(); it != pending_orders.end(); ++it) {
        if (it->id == id) {
            pending_orders.erase(it);
            return;
        }
    }
    for (auto it = active_orders.begin(); it != active_orders.end(); ++it) {
        if (it->id == id) {
            it->finalize(last_ts, last_price, ExitStatus::Cancel);
            completed_orders.push_back(*it);
            active_orders.erase(it);
            return;
        }
    }
}

LMarketReport LMarket::report() const {
    LMarketReport report;
    report.total_orders = completed_orders.size() + active_orders.size() + pending_orders.size();
    report.completed_orders = completed_orders.size();
    report.active_orders = active_orders.size();
    report.pending_orders = pending_orders.size();
    
    for (const auto &order : completed_orders) {
        report.total_profit += order.profit;
        report.total_duration += order.duration;
    }
    
    return report;
}


void LMarketReport::print() const {
    cout << "Market Report:" << endl;
    cout << "Total Orders: " << total_orders << endl;
    cout << "Completed Orders: " << completed_orders << endl;
    cout << "Active Orders: " << active_orders << endl;
    cout << "Pending Orders: " << pending_orders << endl;
    cout << "Total Profit: " << total_profit << " pips" << endl;
    cout << "Total Duration: " << total_duration << " seconds" << endl;
}



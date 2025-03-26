#include "trade.hpp"
#include <sstream>

TradeOriginal::TradeOriginal(const std::string & line) {
    // id,price,qty,quote_qty,time,is_buyer_maker
    // 5663874223,96475.2,0.005,482.376,1733011205575,false
    std::stringstream ss(line);
    std::string token;
    std::getline(ss, token, ',');
    std::getline(ss, token, ',');
    p = std::stod(token);
    std::getline(ss, token, ',');
    v = std::stod(token);
    std::getline(ss, token, ',');
    // q = std::stod(token); --> Not reliable
    q = p * v;
    std::getline(ss, token, ',');
    ts = std::stoll(token);
    std::getline(ss, token, ',');
    is_buyer_maker = token == "true";
    l = 0;
}

size_t TradeOriginal::set_level(PipLevelizer & levelizer) {
    l = levelizer.get_level_binary_search(p);
    return l;
}

std::ostream& operator<<(std::ostream& os, const TradeOriginal& trade) {
    os << "Trade Original:  ts=" << trade.ts << ", p=" << trade.p << ", v=" << trade.v << ", q=" << trade.q << ", is_buyer_maker=" << trade.is_buyer_maker << ", l=" << trade.l;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Trade& trade) {
    os << "Trade: ts=" << trade.ts << ", ts_last=" << trade.ts_last << ", v=" << trade.v << ", q=" << trade.q << ", vs=" << trade.vs << ", vb=" << trade.vb << ", qs=" << trade.qs << ", qb=" << trade.qb << ", l=" << trade.l;
    return os;
}


void Trade::append_trade_original(const TradeOriginal & trade_original) {
    if (l == 0) {
        l = trade_original.l;
    }
    else if (l != trade_original.l) {
        std::cerr << "Trade::append_trade_original: l != trade_original.l: " << l << " != " << trade_original.l << std::endl;
    }
    if (ts == 0) {
        ts = trade_original.ts;
    }
    if (trade_original.ts >= ts_last) {
        ts_last = trade_original.ts;
    } else {
        std::cerr << "Trade::append_trade_original: trade_original.ts < ts_last: " << trade_original.ts << " < " << ts_last << std::endl;
    }
    v += trade_original.v;
    q += trade_original.q;
    if (trade_original.is_buyer_maker) {
        vs += trade_original.v;
        qs += trade_original.q;
    } else {
        vb += trade_original.v;
        qb += trade_original.q;
    }
    duration = ts_last - ts;
}

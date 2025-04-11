#include "vols.hpp"
#include <iostream>
#include <vector>


using namespace std;

Vols::Vols(BinanceCandles &candles, int n) {
    if (candles.size() < n) {
        return;
    }
    double v_acc = 0;
    double b_acc = 0;
    double s_acc = 0;

    for (int i = 0; i < n; i++) {
        v_acc += candles[i].v;
        b_acc += candles[i].vb;
        s_acc += candles[i].vs;
    }
    v.push_back(v_acc / n);
    b.push_back(b_acc / n);
    s.push_back(s_acc / n);
    ts.push_back(candles[n-1].ts);

    for (int i = n; i < candles.size(); i++) {
        v_acc += candles[i].v - candles[i - n].v;
        b_acc += candles[i].vb - candles[i - n].vb;
        s_acc += candles[i].vs - candles[i - n].vs;
        v.push_back(v_acc / n);
        b.push_back(b_acc / n);
        s.push_back(s_acc / n);
        ts.push_back(candles[i].ts);
    }

    // vols has size of candles.size() - n + 1
}

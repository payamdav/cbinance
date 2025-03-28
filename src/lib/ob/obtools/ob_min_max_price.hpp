#pragma once
#include "../ob.hpp"

class OBMinMaxPrice : public OB {
    public:
        double min_price;
        double max_price;

        OBMinMaxPrice(string symbol);

        void on_after_update() override;
        void save_min_max_price();
};


bool read_min_max_price(string symbol, double &min_price, double &max_price);

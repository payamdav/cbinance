#pragma once

#include <vector>
#include <iostream>


class PipLevelizer {
    public:
        double start_price;
        double end_price;
        double percent;
        std::vector<double> levels;

        PipLevelizer(double start_price=10000, double end_price=150000, double percent=0.0001);
        size_t get_level(const double & price);
        size_t get_level_binary_search(const double & price);
        size_t operator () (const double & price);
        bool is_in_range(const double & price);

};

std::ostream& operator<<(std::ostream& os, const PipLevelizer& pip_levelizer);

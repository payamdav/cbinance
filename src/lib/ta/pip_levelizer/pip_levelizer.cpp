#include "pip_levelizer.hpp"
#include <iostream>
#include "../../config/config.hpp" // Include the config header for accessing configuration settings


using namespace std;


PipLevelizer::PipLevelizer(double start_price, double end_price, double percent) {
    this->start_price = start_price;
    this->end_price = end_price;
    this->percent = percent;
    build(); // Build the levels based on the provided parameters
}

PipLevelizer::PipLevelizer(string symbol) {
    MarketInfo market_info;
    double min_price, max_price;
    market_info.get_min_max_price(symbol, min_price, max_price); // Get the min and max prices for the symbol
    double margin = config.get_double("pip_levelizer_margin"); // Get the margin from the configuration
    this->start_price = min_price * (1.0 - margin); // Set the start price with a margin below the min price
    this->end_price = max_price * (1.0 + margin); // Set the end price with a margin above the max price
    this->percent = 0.0001; // Default percentage increment for pip levels, can be adjusted as needed
    build(); // Build the levels based on the calculated start and end prices
}

void PipLevelizer::build() {
    this->levels.clear(); // Clear the existing levels
    double current_price = start_price;
    while (current_price < end_price) {
        this->levels.push_back(current_price);
        current_price *= (1 + percent);
    }
    this->end_price = current_price;
}

std::ostream& operator<<(std::ostream& os, const PipLevelizer& pip_levelizer) {
    os << "PipLevelizer -> Start: " << pip_levelizer.start_price << " -  End: " << pip_levelizer.end_price << " -  Percent: " << pip_levelizer.percent << " -  Levels Count: " << pip_levelizer.levels.size() << std::endl;
    return os;
}

size_t PipLevelizer::get_level(const double & price) {
    for (size_t i = this->levels.size() - 1; i >= 0; i--) {
        if (price >= this->levels[i]) {
            return i;
        }
    }
    return 0;
}

size_t PipLevelizer::get_level_binary_search(const double & price) {
    size_t left = 0;
    size_t right = this->levels.size() - 1;
    // Handle edge cases
    if (price < this->levels[left]) {
        return left; // Return the first level if price is below the first level
    }
    if (price >= this->levels[right]) {
        return right; // Return the last level if price is above or equal to the last level
    }
    // Binary search for the level
    while (left < right - 1) {
        size_t mid = left + (right - left) / 2; // Prevents overflow
        if (this->levels[mid] > price) {
            right = mid - 1; // Move the right pointer to mid - 1 if mid level is greater than price
        } else {
            left = mid; // Move the left pointer to mid if mid level is less than or equal to price
        }
    }
    for (size_t i = right; i >= left; i--) { // Check the last two levels to ensure we find the correct level
        if (this->levels[i] <= price) {
            return i; // Update left to the highest level that is less than or equal to price
        }
    }
}

size_t PipLevelizer::operator () (const double & price) {
    return this->get_level(price);
}

bool PipLevelizer::is_in_range(const double & price) {
    return (price >= this->start_price && price < this->end_price);
}

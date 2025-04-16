#include "anomalies.hpp"
#include "basic_statistics.hpp"
#include <cmath>
#include <algorithm>
#include <ranges>
#include <fstream>
#include "../config/config.hpp"


vector<long> anomalies_by_zscore_of_iqr(const vector<double>& data, size_t idx1, size_t idx2, double zscore_threshold) {
    vector<long> anomalies;  // Vector to store indices of anomalies
    vector<double> ds(data.begin() + idx1, data.begin() + idx2);  // Slice the data from idx1 to idx2
    ranges::sort(ds);  // Sort the sliced data
    double q1 = ds[ds.size() / 4];  // First quartile
    double q3 = ds[ds.size() * 3 / 4];  // Third quartile
    double iqr = q3 - q1;  // Interquartile range
    double lower_bound = q1 - 1.5 * iqr;  // Lower bound for outliers
    double upper_bound = q3 + 1.5 * iqr;  // Upper bound for outliers
    double mean_value = mean(ds);  // Mean of the sliced data
    double std_dev_value = std_dev(ds);  // Standard deviation of the sliced data
    for (size_t i = idx1; i < idx2; i++) {
        double zscore = (data[i] - mean_value) / std_dev_value;  // Calculate z-score
        if (zscore > zscore_threshold || data[i] < lower_bound || data[i] > upper_bound) {
            anomalies.push_back(i);  // If z-score is above threshold or value is outside bounds, add index to anomalies
        }
    }
}
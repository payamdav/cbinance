#pragma once
#include <vector>
#include <utility>
#include <string>
#include <ranges>


using namespace std;

vector<long> anomalies_by_zscore_of_iqr(const vector<double>& data, size_t idx1, size_t idx2, double zscore_threshold);

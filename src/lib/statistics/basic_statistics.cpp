#include "basic_statistics.hpp"


vector<double> i2d(const vector<int>& data) {
    vector<double> d{data.begin(), data.end()};
    return d;
}

double sum(const vector<double>& data) {
    double sum = 0;
    for (double d : data) {
        sum += d;
    }
    return sum;
}

double mean(const vector<double>& data) {
    return sum(data) / data.size();
}

double variance(const vector<double>& data) {
    double m = mean(data);
    double sum = 0;
    for (double d : data) {
        sum += pow(d - m, 2);
    }
    return sum / data.size();
}

double std_dev(const vector<double>& data) {
    return sqrt(variance(data));
}

double median(const vector<double>& data) {
    vector<double> sorted_data = data;
    sort(sorted_data.begin(), sorted_data.end());
    if (sorted_data.size() % 2 == 0) {
        return (sorted_data[sorted_data.size() / 2 - 1] + sorted_data[sorted_data.size() / 2]) / 2;
    } else {
        return sorted_data[sorted_data.size() / 2];
    }
}

double max(const vector<double>& data) {
    double max = data[0];
    for (double d : data) {
        if (d > max) {
            max = d;
        }
    }
    return max;
}

double min(const vector<double>& data) {
    double min = data[0];
    for (double d : data) {
        if (d < min) {
            min = d;
        }
    }
    return min;
}

double range(const vector<double>& data) {
    return max(data) - min(data);
}

vector<double> quantiles(const vector<double>& data, int q) {
    vector<double> sorted_data = data;
    sort(sorted_data.begin(), sorted_data.end());
    vector<double> quantiles;
    for (int i = 0; i < q; i++) {
        quantiles.push_back(sorted_data[sorted_data.size() * i / q]);
    }
    return quantiles;
}


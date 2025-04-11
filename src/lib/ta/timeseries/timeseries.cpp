#include "timeseries.hpp"


size_t TimeSeries::forward_search_ge(size_t t) {
    while (idx < ts.size() && ts[idx] < t) {
        idx++;
    }
    return idx;
}

size_t TimeSeries::forward_search_le(size_t t) {
    while (idx < ts.size() && ts[idx] <= t) {
        idx++;
    }
    if (ts[idx] > t) {
        idx--;
    }
    return idx;
}

size_t TimeSeries::forward_search_g(size_t t) {
    while (idx < ts.size() && ts[idx] <= t) {
        idx++;
    }
    return idx;
}

size_t TimeSeries::forward_search_l(size_t t) {
    while (idx < ts.size() && ts[idx] < t) {
        idx++;
    }
    if (ts[idx] >= t) {
        idx--;
    }
    return idx;
}

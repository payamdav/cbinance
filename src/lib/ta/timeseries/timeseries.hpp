#pragma once
#include <vector>

using namespace std;


class TimeSeries {
    public:
        vector<size_t> ts;
        size_t idx = 0;

        virtual size_t forward_search_ge(size_t t);
        virtual size_t forward_search_le(size_t t);
        virtual size_t forward_search_g(size_t t);
        virtual size_t forward_search_l(size_t t);



};

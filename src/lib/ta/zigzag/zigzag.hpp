#pragma once
#include <vector>
#include "../../trade/tl.hpp"
#include <string>
#include <iostream>


using namespace std;

class Zig_TL {
    public:
        size_t t = 0;
        size_t l = 0;
        bool h = false;

        Zig_TL(size_t t = 0, size_t l = 0, bool h = false);
};

struct ZigZagStat {
    int count = 0;
    int count_h = 0;
    int count_l = 0;
    int avg_l = 0;
    int avg_t = 0;
    int avg_l_h = 0;
    int avg_t_h = 0;
    int avg_l_l = 0;
    int avg_t_l = 0;
};

ostream & operator<<(ostream &os, const ZigZagStat & stat);


class ZigZag_TL : public std::vector<Zig_TL> {
    private:
        Zig_TL first_low;
        Zig_TL first_high;
    public:
        size_t d = 1;

        bool update_in_last_push = false;
        bool append_in_last_push = false;

        ZigZag_TL(size_t delta);

        ZigZag_TL * push(size_t t, size_t l);
        ZigZag_TL * push(const TL &);
        ZigZag_TL * push(const TLS &);
        ZigZag_TL * reset();
        ZigZag_TL * save(const std::string & filename);

        ZigZagStat stat(size_t ts1=0, size_t ts2=2000000000000);
};

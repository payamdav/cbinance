#pragma once
#include <vector>
#include "../../trade/tl.hpp"
#include <string>


using namespace std;

class Zig_TL {
    public:
        size_t t = 0;
        size_t l = 0;
        bool h = false;

        Zig_TL(size_t t = 0, size_t l = 0, bool h = false);
};

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
};

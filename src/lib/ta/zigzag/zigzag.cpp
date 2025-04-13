#include "zigzag.hpp"
#include <fstream>
#include <iostream>
#include "../../config/config.hpp"


Zig_TL::Zig_TL(size_t t, size_t l, bool h) {
    this->t = t;
    this->l = l;
    this->h = h;
}


ZigZag_TL::ZigZag_TL(size_t delta) : d(delta) , first_low(0, 0, false), first_high(0, 0, true) {

}


ZigZag_TL * ZigZag_TL::push(size_t t, size_t l) {
    update_in_last_push = false;
    append_in_last_push = false;
    if (empty()) {
        if (first_low.t == 0) {
            first_low.t = t;
            first_low.l = l;
        }
        if (first_high.t == 0) {
            first_high.t = t;
            first_high.l = l;
        }
        if (l > first_high.l) {
            first_high.t = t;
            first_high.l = l;
        }
        if (l < first_low.l) {
            first_low.t = t;
            first_low.l = l;
        }
        if (first_high.l - first_low.l >= d) {
            if (first_high.t < first_low.t) {
                push_back(first_high);
                push_back(first_low);
                append_in_last_push = true;
            } else if (first_low.t < first_high.t) {
                push_back(first_low);
                push_back(first_high);
                append_in_last_push = true;
            }
        }
    }
    else {
        Zig_TL & last = back();
        if (last.h) {
            if (l > last.l) {
                last.t = t;
                last.l = l;
                update_in_last_push = true;
            } else if (last.l - l >= d) {
                emplace_back(t, l, false);
                append_in_last_push = true;
            }
        } else {
            if (l < last.l) {
                last.t = t;
                last.l = l;
                update_in_last_push = true;
            } else if (l - last.l >= d) {
                emplace_back(t, l, true);
                append_in_last_push = true;
            }
        }
    }

    return this;
}

ZigZag_TL * ZigZag_TL::push(const TL &tl) {
    push(tl.t, tl.l);
    return this;
}

ZigZag_TL * ZigZag_TL::push(const TLS &tls) {
    for (const TL & tl : tls) push(tl);
    return this;
}

ZigZag_TL * ZigZag_TL::reset() {
    clear();
    first_low.t = 0;
    first_low.l = 0;
    first_low.h = false;
    first_high.t = 0;
    first_high.l = 0;
    first_high.h = true;
    update_in_last_push = false;
    append_in_last_push = false;
    return this;
}

ZigZag_TL * ZigZag_TL::save(const std::string & filename) {
    ofstream file(config.get_path("data_path") + "files/" + filename, ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
    // file.write(reinterpret_cast<const char*>(this->data()), size() * sizeof(Zig_TL));
    for (const Zig_TL & zig : *this) {
        file.write(reinterpret_cast<const char*>(&zig.t), sizeof(zig.t));
        file.write(reinterpret_cast<const char*>(&zig.l), sizeof(zig.l));
        file.write(reinterpret_cast<const char*>(&zig.h), sizeof(zig.h));
    }

    return this;
}


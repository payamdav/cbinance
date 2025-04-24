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

ZigZagStat ZigZag_TL::stat(size_t ts1, size_t ts2)  {
    size_t sum_l_h = 0;
    size_t sum_t_h = 0;
    size_t count_h = 0;
    size_t sum_l_l = 0;
    size_t sum_t_l = 0;
    size_t count_l = 0;
    size_t sum_l = 0;
    size_t sum_t = 0;
    size_t count = 0;

    for (int i = 1; i < size(); i++) {
        Zig_TL & zig = at(i);
        if (zig.t >= ts1 && zig.t <= ts2) {
            Zig_TL & prev = at(i - 1);
            count++;
            sum_t += (zig.t - prev.t);
            if (zig.h) {
                sum_l_h += (zig.l - prev.l);
                sum_t_h += (zig.t - prev.t);
                count_h++;
                sum_l += (zig.l - prev.l);
            }
            else {
                sum_l_l += (prev.l - zig.l);
                sum_t_l += (zig.t - prev.t);
                count_l++;
                sum_l += (prev.l - zig.l);
            }
        }
    }

    ZigZagStat stat;
    stat.count = count;
    stat.count_h = count_h;
    stat.count_l = count_l;
    if (count_h > 0) {
        stat.avg_l_h = sum_l_h / count_h;
        stat.avg_t_h = sum_t_h / count_h;
    }
    if (count_l > 0) {
        stat.avg_l_l = sum_l_l / count_l;
        stat.avg_t_l = sum_t_l / count_l;
    }
    if (count > 0) {
        stat.avg_l = sum_l / count;
        stat.avg_t = sum_t / count;
    }
    
    return stat;
}

ostream & operator<<(ostream &os, const ZigZagStat & stat) {
    os << "count: " << stat.count << endl;
    os << "count_h: " << stat.count_h << endl;
    os << "count_l: " << stat.count_l << endl;
    os << "avg_l: " << stat.avg_l << endl;
    os << "avg_t: " << stat.avg_t << endl;
    os << "avg_l_h: " << stat.avg_l_h << endl;
    os << "avg_t_h: " << stat.avg_t_h << endl;
    os << "avg_l_l: " << stat.avg_l_l << endl;
    os << "avg_t_l: " << stat.avg_t_l << endl;
    return os;
}

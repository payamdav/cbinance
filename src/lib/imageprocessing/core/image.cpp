#include "image.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <ranges>
#include "../../../lib/trade/trade.hpp"
#include "../../../lib/ta/pip_levelizer/pip_levelizer.hpp"
#include "../../../lib/utils/datetime_utils.hpp"
#include "../../../lib/utils/string_utils.hpp"



using namespace std;

ostream& img::operator<<(ostream& os, const img::Image& img) {
    os << "Image: " << img.w << "x" << img.h << endl;
    return os;
}

img::Image::Image(int w, int h) : w(w), h(h) {
    data.resize(w * h, 0.0);
}

img::Image* img::Image::save(const string& filename) {
    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Could not open file " << filename << " for writing." << endl;
        return nullptr;
    }

    // Write PGM header
    file << "P5\n" << w << " " << h << "\n255\n";

    // Write pixel data
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            unsigned char pixel = static_cast<unsigned char>(data[y * w + x] * 255);
            file.write(reinterpret_cast<char*>(&pixel), sizeof(pixel));
        }
    }

    file.close();
    return this;
}

img::Image* img::Image::set_inversed_y() {
    inversed_y = true;
    return this;
}

img::Image* img::Image::set_normal_y() {
    inversed_y = false;
    return this;
}

double& img::Image::operator()(int x, int y) {
    if (inversed_y) {
        y = h - 1 - y; // Inverse y coordinate
    }
    if (x < 0 || x >= w || y < 0 || y >= h) {
        cerr << "Error: Coordinates out of bounds." << endl;
        throw out_of_range("Coordinates out of bounds");
    }
    return data[y * w + x];
}

const double& img::Image::operator()(int x, int y) const {
    if (inversed_y) {
        y = h - 1 - y; // Inverse y coordinate
    }
    if (x < 0 || x >= w || y < 0 || y >= h) {
        cerr << "Error: Coordinates out of bounds." << endl;
        throw out_of_range("Coordinates out of bounds");
    }
    // Return a reference to the pixel value
    return data[y * w + x];
}

img::Image* img::Image::draw_line(int x1, int y1, int x2, int y2, double intensity) {
    // Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x1 >= 0 && x1 < w && y1 >= 0 && y1 < h) {
            (*this)(x1, y1) = intensity;
        }
        if (x1 == x2 && y1 == y2) break;
        int err2 = err * 2;
        if (err2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (err2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
    return this;
}

img::Image* img::Image::draw_horizontal_line(int y, double intensity) {
    for (int x = 0; x < w; ++x) {
        if (y >= 0 && y < h) {
            (*this)(x, y) = intensity;
        }
    }
    return this;
}

img::Image* img::Image::draw_vertical_line(int x, double intensity) {
    for (int y = 0; y < h; ++y) {
        if (x >= 0 && x < w) {
            (*this)(x, y) = intensity;
        }
    }
    return this;
}

img::Image* img::Image::normalize(double min, double max) {
    double current_min = *min_element(data.begin(), data.end());
    double current_max = *max_element(data.begin(), data.end());

    for (auto& pixel : data) {
        pixel = (pixel - current_min) / (current_max - current_min) * (max - min) + min;
    }
    return this;
}
img::Image* img::Image::clip_intensity(double min, double max) {
    for (auto& pixel : data) {
        if (pixel < min) {
            pixel = min;
        } else if (pixel > max) {
            pixel = max;
        }
    }
    return this;
}


img::Image* img::Image::load_trades_no_volume(const string& symbol, size_t ts1, size_t ts2, size_t t_interval) {
    // load trades and create image from that
    string sym = utils::toLowerCase(symbol);
    Trades trades(sym);
    trades.open();
    trades.read_by_ts(ts1, ts2);
    trades.close();
    cout << "Trades loaded: " << trades.size() << endl;
    PipLevelizer levelizer(sym);
    double min_price = trades[0].p;
    double max_price = trades[0].p;
    for (const auto& trade : trades) {
        if (trade.p < min_price) {
            min_price = trade.p;
        }
        if (trade.p > max_price) {
            max_price = trade.p;
        }
    }
    double l1 = levelizer(min_price);
    double l2 = levelizer(max_price);
    cout << "Min price: " << min_price << " Max price: " << max_price << " - Min level: " << l1 << " Max level: " << l2 << endl;
    size_t t1 = ((size_t) ts1 / t_interval) * t_interval;
    size_t t2 = ((size_t) ts2 / t_interval) * t_interval;
    size_t w = (t2 - t1) / t_interval + 1;
    size_t h = l2 - l1 + 1;
    Image* img = new Image(w, h);
    img->t_interval = t_interval;
    img->t_offset = t1;
    img->l_offset = l1;
    img->set_inversed_y();
    for (const auto& trade : trades) {
        size_t x = (trade.t - t1) / t_interval;
        size_t y = levelizer(trade.p) - l1;
        (*img)(x, y) = 1.0; // Set pixel to white
    }
    img->set_normal_y();
    return img;
}

img::Image* img::Image::load_trades(const string& symbol, size_t ts1, size_t ts2, size_t t_interval) {
    // load trades and create image from that
    string sym = utils::toLowerCase(symbol);
    Trades trades(sym);
    trades.open();
    trades.read_by_ts(ts1, ts2);
    trades.close();
    cout << "Trades loaded: " << trades.size() << endl;
    PipLevelizer levelizer(sym);
    double min_price = trades[0].p;
    double max_price = trades[0].p;
    for (const auto& trade : trades) {
        if (trade.p < min_price) {
            min_price = trade.p;
        }
        if (trade.p > max_price) {
            max_price = trade.p;
        }
    }
    double l1 = levelizer(min_price);
    double l2 = levelizer(max_price);
    cout << "Min price: " << min_price << " Max price: " << max_price << " - Min level: " << l1 << " Max level: " << l2 << endl;
    size_t t1 = ((size_t) ts1 / t_interval) * t_interval;
    size_t t2 = ((size_t) ts2 / t_interval) * t_interval;
    size_t w = (t2 - t1) / t_interval + 1;
    size_t h = l2 - l1 + 1;
    Image* img = new Image(w, h);
    img->t_interval = t_interval;
    img->t_offset = t1;
    img->l_offset = l1;
    img->set_inversed_y();
    for (const auto& trade : trades) {
        size_t x = (trade.t - t1) / t_interval;
        size_t y = levelizer(trade.p) - l1;
        (*img)(x, y) += trade.v;
    }
    img->set_normal_y();
    img->normalize(0, 1);
    return img;
}

img::Image* img::Image::add(double value, bool nonzero) {
    for (auto& pixel : data) {
        if (nonzero && pixel < eps) {
            continue;
        }
        pixel += value;
    }
    return this;
}

img::Image* img::Image::multiply(double value, bool nonzero) {
    for (auto& pixel : data) {
        if (nonzero && pixel < eps) {
            continue;
        }
        pixel *= value;
    }
    return this;
}


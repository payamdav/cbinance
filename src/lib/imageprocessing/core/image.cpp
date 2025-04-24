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

const double& img::Image::operator()(int x, int y, double default_value) const {
    if (inversed_y) {
        y = h - 1 - y; // Inverse y coordinate
    }
    if (x < 0 || x >= w || y < 0 || y >= h) {
        return default_value;
    }
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

img::Image* img::Image::zero_out(double less_than) {
    for (auto& pixel : data) {
        if (pixel < less_than) {
            pixel = 0.0;
        }
    }
    return this;
}

img::Image* img::Image::ones(int w, int h) {
    Image* img = new Image(w, h);
    fill(img->data.begin(), img->data.end(), 1.0);
    return img;
}

img::Image* img::Image::gaussian_filter(int w, int h, double sigma) {
    Image* img = new Image(w, h);
    double sum = 0.0;
    int half_w = w / 2;
    int half_h = h / 2;

    for (int y = -half_h; y <= half_h; ++y) {
        for (int x = -half_w; x <= half_w; ++x) {
            double value = exp(-(x * x + y * y) / (2 * sigma * sigma));
            (*img)(x + half_w, y + half_h) = value;
            sum += value;
        }
    }

    // Normalize the filter
    for (int i = 0; i < w * h; ++i) {
        img->data[i] /= sum;
    }
    return img;
}

img::Image* img::Image::box_filter(int w, int h) {
    Image* out = Image::ones(w, h);
    out->multiply(1.0 / (w * h));
    return out;
}

img::Image* img::Image::load_trades(const string& symbol, size_t ts1, size_t ts2, size_t t_interval, VolumeType volume_type) {
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
        if (volume_type == VolumeType::BUY && !trade.is_buyer_maker) {
            (*img)(x, y) += trade.v;
        } else if (volume_type == VolumeType::SELL && trade.is_buyer_maker) {
            (*img)(x, y) += trade.v;
        } else if (volume_type == VolumeType::ALL) {
            (*img)(x, y) += trade.v;
        } else if (volume_type == VolumeType::NONE) {
            (*img)(x, y) = 1.0;
        }
    }
    img->set_normal_y();
    img->normalize();
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

img::Image* img::Image::add(const Image& img2, bool nonzero) {
    if (w != img2.w || h != img2.h) {
        cerr << "Error: Images have different dimensions." << endl;
        throw invalid_argument("Images have different dimensions");
    }
    for (int i = 0; i < w * h; ++i) {
        if (nonzero && data[i] < eps) {
            continue;
        }
        data[i] += img2.data[i];
    }
    return this;
}

img::Image* img::Image::transpose() {
    Image * transposed_img = new Image(h, w);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            (*transposed_img)(y, x) = (*this)(x, y);
        }
    }
    transposed_img->t_interval = t_interval;
    transposed_img->t_offset = t_offset;
    transposed_img->l_offset = l_offset;
    return transposed_img;
}

img::Image* img::Image::scale(double xfactor, double yfactor) {
    int new_w = static_cast<int>(w * xfactor);
    int new_h = static_cast<int>(h * yfactor);
    Image * scaled_img = new Image(new_w, new_h);
    for (int y = 0; y < new_h; ++y) {
        for (int x = 0; x < new_w; ++x) {
            int orig_x = static_cast<int>(x / xfactor);
            int orig_y = static_cast<int>(y / yfactor);
            (*scaled_img)(x, y) = (*this)(orig_x, orig_y);
        }
    }
    scaled_img->t_interval = t_interval;
    scaled_img->t_offset = t_offset;
    scaled_img->l_offset = l_offset;
    return scaled_img;
}

img::Image* img::Image::convolve(const Image& kernel) {
    int kernel_w = kernel.w;
    int kernel_h = kernel.h;
    int kernel_center_x = kernel_w / 2;
    int kernel_center_y = kernel_h / 2;

    Image * convolved_img = new Image(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double sum = 0.0;
            for (int ky = 0; ky < kernel_h; ++ky) {
                for (int kx = 0; kx < kernel_w; ++kx) {
                    int img_x = x + kx - kernel_center_x;
                    int img_y = y + ky - kernel_center_y;
                    if (img_x >= 0 && img_x < w && img_y >= 0 && img_y < h) {
                        sum += (*this)(img_x, img_y) * kernel(kx, ky);
                    }
                }
            }
            (*convolved_img)(x, y) = sum;
        }
    }
    convolved_img->t_interval = t_interval;
    convolved_img->t_offset = t_offset;
    convolved_img->l_offset = l_offset;
    return convolved_img;
}

img::Image* img::Image::convolve_valid(const Image& kernel) {
    int kernel_w = kernel.w;
    int kernel_h = kernel.h;
    int kernel_center_x = kernel_w / 2;
    int kernel_center_y = kernel_h / 2;

    Image * convolved_img = new Image(w - kernel_w + 1, h - kernel_h + 1);
    for (int y = 0; y < h - kernel_h + 1; ++y) {
        for (int x = 0; x < w - kernel_w + 1; ++x) {
            double sum = 0.0;
            for (int ky = 0; ky < kernel_h; ++ky) {
                for (int kx = 0; kx < kernel_w; ++kx) {
                    sum += (*this)(x + kx, y + ky) * kernel(kx, ky);
                }
            }
            (*convolved_img)(x, y) = sum;
        }
    }
    convolved_img->t_interval = t_interval;
    convolved_img->t_offset = t_offset + kernel_center_x * t_interval;
    convolved_img->l_offset = l_offset + kernel_center_y;
    return convolved_img;
}
#pragma once
#include <string>
#include <vector>
#include <iostream>

using namespace std;


namespace img {

// Image class - grayscale - 1 channel from 0 to 1

const double eps = 1e-8;


class Image {
    public:
        int w;
        int h;
        vector<double> data;
        bool inversed_y = false;
        size_t t_interval = 0;
        size_t t_offset = 0;
        size_t l_offset = 0;

        Image(int w, int h);
        Image * save(const string& filename);

        Image * set_inversed_y();
        Image * set_normal_y();

        Image * normalize(double min=0, double max=1);
        Image * clip_intensity(double min=0, double max=1);

        Image * add(double value, bool nonzero=false);
        Image * multiply(double value, bool nonzero=false);

        // operator overload () and = for reading and writing
        double& operator()(int x, int y);
        const double& operator()(int x, int y) const;

        Image * draw_line(int x1, int y1, int x2, int y2, double intensity=1);
        Image * draw_horizontal_line(int y, double intensity=1);
        Image * draw_vertical_line(int x, double intensity=1);

        static Image * load_trades_no_volume(const string& symbol, size_t ts1, size_t ts2, size_t t_interval=60000);
        static Image * load_trades(const string& symbol, size_t ts1, size_t ts2, size_t t_interval=60000);

};

ostream& operator<<(ostream& os, const Image& img);

}


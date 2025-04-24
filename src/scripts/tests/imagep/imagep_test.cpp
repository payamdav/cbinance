#include <iostream>
#include <vector>
#include <string>
#include "../../../lib/utils/datetime_utils.hpp"

#include "../../../lib/imageprocessing/core/image.hpp"


using namespace std;
using namespace img;

void test1() {
    // Create a 100x100 image
    Image img(100, 100);

    // Draw a line from (10, 10) to (90, 90)
    img.draw_line(10, 10, 90, 90);
    img.draw_horizontal_line(10);
    img.draw_vertical_line(10);

    // Save the image to a file
    img.save("/home/payam/data/files/test.pgm");

    // Print the image dimensions
    cout << img;

}

void test2() {
    Image * img = Image::load_trades("ADAUSDT", utils::get_timestamp("2025-03-15 00:00:00"), utils::get_timestamp("2025-03-15 23:59:59"), 60000);
    img->add(0.2, true)->normalize(); // to bypass effect of outliers that shrinks all data to near zero and makes it hard to see the data
    img->save("/home/payam/data/files/test.pgm");
    cout << *img << endl;
}

void test3() {
    Image * img = Image::load_trades("ADAUSDT", utils::get_timestamp("2025-03-15 00:00:00"), utils::get_timestamp("2025-03-15 23:59:59"), 60000);
    // img->add(0.2, true)->normalize(); // to bypass effect of outliers that shrinks all data to near zero and makes it hard to see the data
    img->save("/home/payam/data/files/test1.pgm");
    Image * box = Image::box_filter(121, 11);
    Image * box2 = Image::box_filter(31, 5);
    Image * gaus = Image::gaussian_filter(121, 11);
    Image * img2 = img->convolve(*box)->normalize();    
    Image * img3 = img2->convolve(*gaus)->convolve(*gaus)->convolve(*gaus)->normalize();
    // img2->zero_out(0.7)->normalize();
    img3->zero_out(0.5)->normalize();
    img2->save("/home/payam/data/files/test2.pgm");
    img3->save("/home/payam/data/files/test3.pgm");
}

void test4() {
    Image * img = Image::load_trades("ADAUSDT", utils::get_timestamp("2025-03-15 00:00:00"), utils::get_timestamp("2025-03-15 23:59:59"), 60000, img::VolumeType::ALL);
    // img->add(0.2, true)->normalize(); // to bypass effect of outliers that shrinks all data to near zero and makes it hard to see the data
    img->save("/home/payam/data/files/test1.pgm");
    Image * box = Image::box_filter(11, 9);
    // Image * box2 = Image::box_filter(31, 5);
    Image * img2 = img->convolve_valid(*box)->normalize();
    // img2 = img2->scale(100, 1)->normalize();    
    // Image * img3 = img->convolve(*box2)->normalize();
    // img2->zero_out(0.5)->normalize();
    // img3->zero_out(0.5)->normalize();
    img2->save("/home/payam/data/files/test2.pgm");
    // img3->save("/home/payam/data/files/test3.pgm");
}

int main() {
    test4();
    return 0;
}

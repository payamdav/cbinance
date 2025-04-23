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
    // Image * img = Image::load_trades_no_volume("ADAUSDT", utils::get_timestamp("2025-03-15 00:00:00"), utils::get_timestamp("2025-03-15 23:59:59"), 60000);
    Image * img = Image::load_trades("ADAUSDT", utils::get_timestamp("2025-03-15 00:00:00"), utils::get_timestamp("2025-03-15 23:59:59"), 60000);
    img->add(0.2, true)->normalize(); // to bypass effect of outliers that shrinks all data to near zero and makes it hard to see the data
    img->save("/home/payam/data/files/test.pgm");
    cout << *img << endl;
}

int main() {
    test2();
    return 0;
}

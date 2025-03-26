#include <iostream>
#include "../../lib/depth_file/depth_file.hpp"
#include "../../lib/ta/pip_levelizer/pip_levelizer.hpp"

using namespace std;


void process_depth_csv_and_add_to_levelized(string filename, PipLevelizer levelizer) {
    process_file(filename, levelizer);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Please provide the depth file" << endl;
        cout << "depth_file_converter depth_2025-2-26.txt depth_2025-2-27.txt depth_2025-2-28.txt depth_2025-3-1.txt" << endl;
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        string filename = argv[i];
        PipLevelizer levelizer(10000, 150000, 0.0001);
        process_depth_csv_and_add_to_levelized(filename, levelizer);
    }

    return 0;

}

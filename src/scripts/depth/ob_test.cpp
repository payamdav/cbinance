#include <iostream>
#include <string>
// #include <vector>
// #include "../../lib/config/config.hpp"
// #include "../../lib/ob/snapshot.hpp"
// #include "../../lib/ob/update.hpp"
#include "../../lib/ob/ob.hpp"


using namespace std;



void ob_test_1(int argc, char *argv[]) {
    // auto symbols = config.get_csv_strings("symbols_list");
    OB ob("adausdt");
    ob.build();
}




int main(int argc, char *argv[]) {
    ob_test_1(argc, argv);
    
    return 0;
}

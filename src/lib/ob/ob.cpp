#include "ob.hpp"
#include <iostream>
#include <string>



OB::OB(string symbol) : symbol(symbol), snapshot(new Snapshot(symbol)), update(new Update(symbol)), u_id(0), t(0) {
    std::cout << "OB constructor called for: " << symbol << std::endl;
}

OB::~OB() {
    std::cout << "OB destructor called for: " << symbol << std::endl;
    delete snapshot;
    delete update;
}

void OB::build(size_t from_ts, size_t to_ts) {
    cout << "Building OB for symbol: " << symbol << endl;

}

#include <iostream>
#include "cache.hpp"

int main() {
    std::cout << "Cache Simulator" << std::endl;

    Cache cache({64,16,2,Policy::LRU});
    cache.access(0x00);
    cache.access(0x04);

    return 0;
}
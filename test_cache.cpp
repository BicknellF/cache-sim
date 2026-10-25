#include "cache.hpp"
#include <cassert>
#include <iostream>
#include <vector>

int main() {
    const std::vector<std::uint64_t> trace = {0x00, 0x04, 0x10, 0x20,0x00,0x40,0x20,0x00};
    const std::vector<bool> lru_expected = {0,1,0,0,1,0,0,0};
    const std::vector<bool> fifo_expected = {0,1,0,0,1,0,1,0};

    Cache lru({64,16,2,Policy::LRU});
    Cache fifo({64,16,2,Policy::FIFO});
    for (std::size_t i = 0; i < trace.size(); ++i) {
        assert(lru.access(trace[i]) == lru_expected[i]);
        assert(fifo.access(trace[i]) == fifo_expected[i]);
    }
    std::cout << "All tests passed!" << std::endl;
}
#include "cache.hpp"
#include <stdexcept>
#include <random>

// Small helper: check if a number is a power of two.
static bool isPowerOfTwo(std::size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

Cache::Cache(const CacheConfig& cfg) : cfg_(cfg) {
    if (cfg.cacheSize == 0 || cfg.blockSize == 0 || cfg.associativity == 0) {
        throw std::invalid_argument("cache parameters must be non-zero");
    }

    num_sets_ = cfg.cacheSize / (cfg.blockSize * cfg.associativity);
    if (num_sets_ == 0) {
        throw std::invalid_argument("cache too small for one set");
    }

    if (!isPowerOfTwo(cfg.blockSize) || !isPowerOfTwo(num_sets_)) {
        throw std::invalid_argument("block size and number of sets must be powers of two");
    }

    lines_.resize(num_sets_ * cfg.associativity);


    offset_bits_ = 0;
    std::size_t b = cfg.blockSize;
    while (b > 1) { b = b / 2; offset_bits_++; }

    index_bits_ = 0;
    std::size_t s = num_sets_;
    while (s > 1) { s = s / 2; index_bits_++; }

    tag_bits_ = 64 - offset_bits_ - index_bits_;
}

bool Cache::access(std::uint64_t addr) {
    std::uint64_t index = (addr >> offset_bits_) & (num_sets_ - 1);
    std::uint64_t tag   = addr >> (offset_bits_ + index_bits_);

    //start and end of set in lines_
    std::size_t start = index * cfg_.associativity;
    std::size_t end   = start + cfg_.associativity;

    // hit?
    for (std::size_t i = start; i < end; i++) {
        if (lines_[i].valid && lines_[i].tag == tag) {
            stats_.hits++;
            lines_[i].lastAccessTime = ++clock_;
            return true;
        }
    }

    // Miss
    stats_.misses++;
    std::size_t victim = start;

    bool foundEmpty = false;
    for (std::size_t i = start; i < end; i++) {
        if (!lines_[i].valid) {
            victim = i;
            foundEmpty = true;
            break;
        }
    }

    if (!foundEmpty) {
        if (cfg_.replacementPolicy == Policy::LRU) {
            std::uint64_t minTime = lines_[start].lastAccessTime;
            for (std::size_t i = start + 1; i < end; i++) {
                if (lines_[i].lastAccessTime < minTime) {
                    minTime = lines_[i].lastAccessTime;
                    victim = i;
                }
            }
        } else if (cfg_.replacementPolicy == Policy::FIFO) {
            std::uint64_t minTime = lines_[start].insertionTime;
            for (std::size_t i = start + 1; i < end; i++) {
                if (lines_[i].insertionTime < minTime) {
                    minTime = lines_[i].insertionTime;
                    victim = i;
                }
            }
        } else if (cfg_.replacementPolicy == Policy::Random) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<std::size_t> dis(0, cfg_.associativity - 1);
            victim = start + dis(gen);
        } else {
            throw std::invalid_argument("Invalid replacement policy");
        }
    }

    lines_[victim].valid = true;
    lines_[victim].tag = tag;
    lines_[victim].insertionTime = ++clock_;
    lines_[victim].lastAccessTime = clock_;

    return false;
}
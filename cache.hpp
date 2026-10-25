#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

enum class Policy { LRU, FIFO, Random };

struct CacheConfig {
    std::size_t cacheSize;
    std::size_t blockSize;
    std::size_t associativity;
    Policy replacementPolicy;
};

struct CacheStats {
    std::uint64_t hits = 0;
    std::uint64_t misses = 0;
};

class Cache {
public:
    explicit Cache(const CacheConfig& cfg);
    bool access(std::uint64_t addr);
    const CacheStats& stats() const { return stats_; }

private:
    struct Line {
        bool valid = false; // False if the line is empty, true if it contains data
        std::uint64_t tag = 0;
        std::uint64_t lastAccessTime = 0;
        std::uint64_t insertionTime = 0;
    };

    CacheConfig cfg_;
    std::vector<Line> lines_;
    std::size_t num_sets_ = 0;
    unsigned offset_bits_ = 0;
    unsigned index_bits_ = 0;
    unsigned tag_bits_ = 0;
    CacheStats stats_;
    std::uint64_t clock_ = 0;
};
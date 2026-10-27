#include "cache.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

const int N = 250;

// Row-friendly: A and C are accessed row by row.
void mulRowFriendly(const std::vector<double>& A,
                    const std::vector<double>& B,
                    std::vector<double>& C, int n) {
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            double a = A[i * n + k];
            for (int j = 0; j < n; j++) {
                C[i * n + j] += a * B[k * n + j];
            }
        }
    }
}

// Column friendly C and A
void mulColFriendly(const std::vector<double>& A,
                    const std::vector<double>& B,
                    std::vector<double>& C, int n) {
    for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
            double b = B[k * n + j];
            for (int i = 0; i < n; i++) {
                C[i * n + j] += A[i * n + k] * b;
            }
        }
    }
}

void resetMatrix(std::vector<double>& C, int n) {
    for (int i = 0; i < n * n; i++) {
        C[i] = 0.0;
    }
}

void simRow(Cache& cache, int n) {
    const std::uint64_t A0 = 0;
    const std::uint64_t B0 = (1ULL << 20);
    const std::uint64_t C0 = (2ULL << 20);
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            std::uint64_t offsetA = (i * n + k) * 8;
            cache.access(A0 + offsetA);
            for (int j = 0; j < n; j++) {
                std::uint64_t offsetB = (k * n + j) * 8;
                std::uint64_t offsetC = (i * n + j) * 8;
                cache.access(B0 + offsetB);
                cache.access(C0 + offsetC);
            }
        }
    }
}

void simCol(Cache& cache, int n) {
    const std::uint64_t A0 = 0;
    const std::uint64_t B0 = (1ULL << 20);
    const std::uint64_t C0 = (2ULL << 20);
    for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
            std::uint64_t offsetB = (k * n + j) * 8;
            cache.access(B0 + offsetB);
            for (int i = 0; i < n; i++) {
                std::uint64_t offsetA = (i * n + k) * 8;
                std::uint64_t offsetC = (i * n + j) * 8;
                cache.access(A0 + offsetA);
                cache.access(C0 + offsetC);
            }
        }
    }
}

double hitRate(const Cache& c) {
    std::uint64_t total = c.stats().hits + c.stats().misses;
    if (total == 0) {
        return 0.0;
    }
    return 100.0 * c.stats().hits / total;
}

void printStats(std::string name, const Cache& c) {
    std::cout << "  " << name << ": hits=" << c.stats().hits
              << " misses=" << c.stats().misses
              << " hit rate=" << hitRate(c) << "%\n";
}

int main() {
    std::cout << "Matrix multiplication access-pattern comparison\n\n";

    // Timing benchmark.
    std::vector<double> A(N * N, 1.0), B(N * N, 1.0), C(N * N, 0.0);

    resetMatrix(C, N);
    auto start = std::chrono::steady_clock::now();
    mulRowFriendly(A, B, C, N);
    auto end = std::chrono::steady_clock::now();
    double tRow = std::chrono::duration<double, std::milli>(end - start).count();

    resetMatrix(C, N);
    start = std::chrono::steady_clock::now();
    mulColFriendly(A, B, C, N);
    end = std::chrono::steady_clock::now();
    double tCol = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "Timing (N=" << N << "):\n";
    std::cout << "  row-friendly (i-k-j): " << tRow << " ms\n";
    std::cout << "  col-friendly (j-k-i): " << tCol << " ms\n";

    // Cache simulation.
    Cache cacheRow({32768, 64, 8, Policy::LRU});
    simRow(cacheRow, N);
    Cache cacheCol({32768, 64, 8, Policy::LRU});
    simCol(cacheCol, N);

    std::cout << "Cache simulation (N=" << N << ", 32KB, 64B blocks, 8-way):\n";
    printStats("row-friendly", cacheRow);
    printStats("col-friendly", cacheCol);

    return 0;
}

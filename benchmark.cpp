#include <iostream>
#include <cstdint>
#include <algorithm>
#include <optional>
#include <chrono>

#include "utils.h"

using std::cout;
using std::endl;

const size_t size = 100'000'000;

int benchmark() {
    cout << endl;

    // alignas(32) int64_t block[8] = { 10, 20, 30, 40, 50, 60, 70, 80 };

    int64_t *block = new (std::align_val_t {32}) int64_t[size];
    int64_t *block2 = new (std::align_val_t {32}) int64_t[size];

    for (size_t i = 0; i < size; ++i) {
      block[i] = i * 10 + 10;
    }

    int32_t target = 100'000;

    auto start = std::chrono::steady_clock::now();
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    int64_t *index0;

    for (int target = 0; target < 10; ++target) {
        cout << "===== " << target * 10000 << endl;

        index0 = std::lower_bound(block, block + size, target * 10000);
        start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < 1000; ++ i) {
            index0 = std::lower_bound(block, block + size, target * 10000);
        }
        stop = std::chrono::steady_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << duration << "\t" << "Target " << target * 10000 << " belongs at index: " << index0 - block << std::endl; 

        index0 = lowerBound(block, size, target * 10000);
        start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < 1000; ++ i) {
            index0 = lowerBound(block, size, target * 10000);
        }
        stop = std::chrono::steady_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << duration << "\t" << "Target " << target * 10000 << " belongs at index: " << index0 - block << std::endl;
    }

    return 0;
}

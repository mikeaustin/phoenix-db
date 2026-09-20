// g++ -std=c++20 -mavx2

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <optional>
#include <chrono>
#include <bit>
#include <immintrin.h>

#include "utils.h"

using std::cout;
using std::endl;

//

inline int search_avx2_block(const int32_t* block, int32_t target) {
    // 1. Broadcast the search target to all 8 lanes of a YMM register
    __m256i keys = _mm256_set1_epi32(target);

    // 2. Load the 8 sorted elements from the block (must be 32-byte aligned for performance)
    __m256i data = _mm256_loadu_si256((const __m256i*)block);

    // 3. Compare: Generates 0xFFFFFFFF where target > data, 0x00000000 otherwise
    __m256i cmp = _mm256_cmpgt_epi32(keys, data);

    // 4. Extract the sign bits of each lane into an 8-bit integer mask
    int mask = _mm256_movemask_ps(_mm256_castsi256_ps(cmp));

    // 5. The number of set bits (popcount) gives exactly how many keys are smaller than 'target'
    // This maps directly to the index of the next child or insertion index.
    return _mm_popcnt_u32(mask);
}

// Returns the index of the first element that does not compare less than target.
// The array MUST be aligned or unaligned-safe, and padded so reading 8 elements is safe.
int64_t simd_lower_bound(const int32_t *array, int64_t size, int32_t target) {
    int64_t low = 0, high = size;

    // Broadcast the target value to all 8 slots
    __m256i v_target = _mm256_set1_epi32(target);

    // Traditional binary search loop, but we operate in blocks of 8 elements
    while (high - low >= 8) {
        // Find a midpoint block aligned to 8 elements
        int64_t mid = low + ((high - low) / 16) * 8; 

        // Load 8 contiguous elements
        __m256i v_array = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(&array[mid]));

        // Compare: returns 0xFFFFFFFF if target > array_elem, else 0x0
        __m256i v_cmp = _mm256_cmpgt_epi32(v_target, v_array);

        // Extract the signs of the 8 floats/ints into an 8-bit mask
        int mask = _mm256_movemask_ps(_mm256_castsi256_ps(v_cmp));

        if (mask == 0xFF) {
            // All 8 elements in this block are less than target. 
            // Move search space past this block.
            low = mid + 8;
        } else if (mask == 0x00) {
            // All 8 elements in this block are greater than or equal to target.
            // Move search space to before this block.
            high = mid;
        } else {
            // The boundary lies inside this 8-element block.
            // Count trailing zeros (or count set bits) to find exactly how many elements are smaller.
            int smaller_count = __builtin_ctz(~mask); 
            
            return mid + smaller_count;
        }
    }

    // Scalar cleanup for remaining elements if the array size wasn't a multiple of 8
    while (low < high) {
        int64_t mid = low + (high - low) / 2;

        if (array[mid] < target) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    return low;
}

// Returns the index of the first element which does not compare less than 'target'.
// 'data' must be a sorted array of int64_t.
size_t avx2_lower_bound_i64(const int64_t* data, size_t size, int64_t target) {
    if (size == 0) return 0;

    // Broadcast the target value to all 4 lanes of a 256-bit register
    __m256i target_vec = _mm256_set1_epi64x(target);
    
    size_t i = 0;
    // Process elements in blocks of 4
    for (; i + 3 < size; i += 4) {
        // Load 4 elements from the array
        __m256i data_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&data[i]));
        
        // Compare target > data_vec elements. 
        // _mm256_cmpgt_epi64 returns 0xFFFFFFFFFFFFFFFF (-1) if target > data, 0 otherwise.
        __m256i cmp_mask = _mm256_cmpgt_epi64(target_vec, data_vec);
        
        // Move the most significant bit of each 8-bit lane to a 32-bit integer mask.
        // Each 64-bit element has 8 bytes. If target > data, all 8 bytes have MSB set.
        int mask = _mm256_movemask_epi8(cmp_mask);
        
        // If the mask is not completely filled with 1s (0xFFFFFFFF), 
        // it means at least one element in this block is >= target.
        if (mask != 0xFFFFFFFF) {
            // Find the exact element within this 4-element block
            if (data[i] >= target)     return i;
            if (data[i + 1] >= target) return i + 1;
            if (data[i + 2] >= target) return i + 2;
            return i + 3;
        }
    }

    // Clean up remaining elements (less than 4 left)
    for (; i < size; ++i) {
        if (data[i] >= target) {
            return i;
        }
    }

    return size;
}

const size_t size = 100'000'000;

int main2() {
    cout << endl;

    // alignas(32) int64_t block2[8] = { 10, 20, 30, 40, 50, 60, 70, 80 };

    int64_t *block2 = new (std::align_val_t {32}) int64_t[size];

    for (size_t i = 0; i < size; ++i) {
      block2[i] = i * 10 + 10;
    }

    int32_t target = 100'000;

    auto start = std::chrono::steady_clock::now();
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    int64_t *index0;
    int index;

    for (int target = 0; target < 10; ++target) {
      cout << "===== " << target * 10000 << endl;

      start = std::chrono::steady_clock::now();
      for (size_t i = 0; i < 100000; ++ i) {
        index0 = std::lower_bound(block2, block2 + size, target * 10000);
      }
      stop = std::chrono::steady_clock::now();
      duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

      std::cout << duration << "\t" << "Target " << target * 10000 << " belongs at index: " << index0 - block2 << std::endl; 

      // start = std::chrono::steady_clock::now();
      // for (size_t i = 0; i < 100000; ++ i) {
      //   index = avx2_lower_bound_i64(block2, size, target * 10000);
      // }
      // stop = std::chrono::steady_clock::now();
      // duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

      // std::cout << duration << "\t" << "Target " << target * 10000 << " belongs at index: " << index << std::endl; 

      start = std::chrono::steady_clock::now();
      for (size_t i = 0; i < 100000; ++ i) {
        index = lowerBound2(block2, size, target * 10000);
      }
      stop = std::chrono::steady_clock::now();
      duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

      std::cout << duration << "\t" << "Target " << target * 10000 << " belongs at index: " << index << std::endl; 
    }

    return 0;
}

//

struct Team {
  Type type;
  uint32_t _pad1;
  uint64_t id;
};

template<int TTitleLength> struct Item {
  Type type;
  uint32_t _pad1;
  uint64_t id;
  uint64_t teamId;
  uint32_t _pad2;
  String<TTitleLength> title;
};

struct Data {
  Team team1 = { TEAM, 0, 1000 };
  Team team2 = { TEAM, 0, 1001 };
  Item<4> item1 = { ITEM, 0, 20000, 1000, 0, { 4, { 'A', 'B', 'C', 0 } } };
  Item<12> item2 = { ITEM, 0, 20001, 1000, 0, { 12, 'D', 'E', 'F', 'G', 'H', 0 } };
  Item<16> item3 = { ITEM, 0, 20002, 1001, 0, { 16, 'I', 'J', 'K', 'L', 0 } };
  Item<10> item4 = { ITEM, 0, 20003, 1001, 0, { 10, 'M', 'N', 'O', 0 } };
} data;

template <typename TType>
struct Index {
  TType value;
  size_t offset;
};

//

Index<uint64_t> indexData[] = {
  { 1000, 0 }, { 1001, 2 },
};

size_t itemTeamOrderedIndex[] = {
  32, 72, 120, 168,
};

Index<uint64_t> itemIdIndex[] = {
  { 20000, 32 }, { 20001, 72 }, { 20002, 120 }, { 20003, 168 },
};

int main() {
  cout << format("Offset", "Type", "ID", "Team ID", "Length", "Title") << endl;
  cout << format("=======", "=======", "=======", "=======", "=======", "=======") << endl;

  int8_t *ptr = reinterpret_cast<int8_t *>(&data);

  while (ptr < reinterpret_cast<int8_t *>(&data) + sizeof(data)) {
    auto type = getInt<uint32_t>(ptr, 0);
    auto id = getInt<uint64_t>(ptr, 8);

    if (type == Type::TEAM) {
      cout << ptr - reinterpret_cast<int8_t *>(&data) << "\t" << type << "\t" << id << endl;

      ptr += sizeof(Team);
    } else if (type == Type::ITEM) {
      auto teamId = getInt<uint64_t>(ptr, 16);
      auto length = getInt<uint32_t>(ptr, 28);
      auto title = getString(ptr, 32);

      cout << ptr - reinterpret_cast<int8_t *>(&data) << "\t" << format(type, id, teamId, length, title) << endl;

      ptr += sizeof(Item<0>) + (length + 8 - 1) / 8 * 8;
    } else {
      ptr = 0;
    }
  }

  cout << endl;

  auto element = binarySearch(itemIdIndex, 20001);

  if (element) {
    int8_t *ptr = reinterpret_cast<int8_t *>(&data) + (*element)->offset;

    cout << getInt<uint32_t>(ptr, 0) << "\t" << getInt<uint64_t>(ptr, 8) << endl;
  }

  //

  auto element2 = binarySearch(indexData, 1000);

  if (element) {
    ptr = reinterpret_cast<int8_t *>(&data) + itemTeamOrderedIndex[(*element2)->offset];

    while (ptr < reinterpret_cast<int8_t *>(&data) + sizeof(data)) {
      auto type = getInt<uint32_t>(ptr, 0);
      auto id = getInt<uint64_t>(ptr, 8);
      auto teamId = getInt<uint64_t>(ptr, 16);
      auto length = getInt<uint32_t>(ptr, 28);
      auto title = getString(ptr, 32);

      if (teamId != 1000) {
        break;
      }

      cout << ptr - reinterpret_cast<int8_t *>(&data) << "\t" << format(type, id, teamId, length, title) << endl;

      ptr += sizeof(Item<0>) + (length + 8 - 1) / 8 * 8;
    }
  }

  main2();

  return 0;
}

// g++ -std=c++20

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <optional>
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

int32_t horizontal_stree_search(const int32_t* tree, size_t num_nodes, int32_t target) {
    size_t node_idx = 0;
    __m256i v_target = _mm256_set1_epi32(target);

    // Navigate down the tree levels
    while (node_idx < num_nodes) {
        // Load all 8 elements of the current node into a SIMD register
        __m256i v_node = _mm256_loadu_si256((const __m256i*)&tree[node_idx * 8]);

        // Compare target against all 8 elements (returns 0xFFFFFFFF if target > element)
        __m256i v_cmp = _mm256_cmpgt_epi32(v_target, v_node);

        // Extract comparison results into an 8-bit mask
        unsigned int mask = _mm256_movemask_ps(_mm256_castsi256_ps(v_cmp));

        // The number of set bits (popcount) gives the number of elements smaller than target
        // int child_branch = _popcnt32(mask); // Range: 0 to 8
        int child_branch = std::popcount(mask); // Works on MSVC, GCC, and Clang safely

        // If target is smaller than all elements, branch is 0. If larger than all, branch is 8.
        // Calculate the next node index
        node_idx = node_idx * 8 + child_branch + 1;
    }

    // Leaf fixup and indexing mapping back to sorted array would go here...
    return -1; 
}

int main2() {
    // A sample 32-byte aligned sorted block of 8 integers
    alignas(32) int32_t block[8] = { 10, 20, 30, 40, 50, 60, 70, 80 };

    int32_t target = 45;
    int index = search_avx2_block(block, target);

    int index2 = horizontal_stree_search(block, 1, target);

    std::cout << "Target " << target << " belongs at index: " << index << std::endl; 
    // Output will be 4 (points to 50, since 10,20,30,40 are smaller)

    std::cout << "Target " << target << " belongs at index: " << index2 << std::endl; 

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

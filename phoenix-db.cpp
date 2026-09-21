// g++ -std=c++20 -mavx2

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <optional>
#include <chrono>
#include <bits/stdc++.h>

#include "utils.h"

using std::cout;
using std::endl;

//

const size_t size = 100'000'000;

int main2() {
    cout << endl;

    // alignas(32) int64_t block[8] = { 10, 20, 30, 40, 50, 60, 70, 80 };

    int64_t *block = new (std::align_val_t {32}) int64_t[size];
    int64_t *block2 = new (std::align_val_t {32}) int64_t[size];

    for (size_t i = 0; i < size; ++i) {
      block[i] = i * 10 + 10;
    }

    eytzinger_init(block, block2, size);

    int32_t target = 100'000;

    auto start = std::chrono::steady_clock::now();
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    int64_t *index0;
    int index;

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

      index = lowerBound(block, size, target * 10000);
      start = std::chrono::steady_clock::now();
      for (size_t i = 0; i < 1000; ++ i) {
        index = lowerBound(block, size, target * 10000);
      }
      stop = std::chrono::steady_clock::now();
      duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

      std::cout << duration << "\t" << "Target " << target * 10000 << " belongs at index: " << index << std::endl;

      // start = std::chrono::steady_clock::now();
      // for (size_t i = 0; i < 100000; ++ i) {
      //   index = eytzinger_lower_bound(block2, size, target *10000);
      // }
      // stop = std::chrono::steady_clock::now();
      // duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

      // std::cout << duration << "\t" << "Target " << target * 10000 << " belongs at index: " << index << std::endl;
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

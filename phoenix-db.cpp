// g++ -std=c++20 -mavx2

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <optional>
#include <chrono>

#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;

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

// Unique key
uint64_t itemIdIndex[] = {
    20000, 20001, 20002, 20003,
};

uint64_t itemIdIndexData[] = {
    32, 72, 120, 168,
};

//

uint64_t itemTeamIdIndex2[] = {
    1000, 1001,
};

uint16_t itemTeamIdIndex2Data[] = {
  0, 120,
};

// Index deduplication
Index<uint64_t> itemTeamIdIndex[] = {
    { 1000, 0 }, { 1001, 2 },
};

// Sorted by teamId, sortOrder
size_t itemTeamIdIndexData[] = {
    32, 72, 120, 168,
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

    auto index = lowerBound(itemIdIndex, sizeof(itemIdIndex) / sizeof(uint64_t), (uint64_t) 20001);

    if (index) {
        int8_t *ptr = reinterpret_cast<int8_t *>(&data) + itemIdIndexData[*index];

        cout << getInt<uint32_t>(ptr, 0) << "\t" << getInt<uint64_t>(ptr, 8) << endl;
    } else {
        cerr << "Item not found" << endl;
    }

    //

    auto element2 = binarySearch(itemTeamIdIndex, 1000);

    if (element2) {
        ptr = reinterpret_cast<int8_t *>(&data) + itemTeamIdIndexData[(*element2)->offset];

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

    // benchmark();

    return 0;
}

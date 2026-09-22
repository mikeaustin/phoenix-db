// g++ -std=c++20 -O3 -flto

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <optional>
#include <chrono>

#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;

Field teamFields[] = {
    { Primitive::TYPE32, "type" },
    { Primitive::TYPE32, "_padding" },
    { Primitive::UINT64, "id" },
    { },
};

Field itemFields[] = {
    { Primitive::TYPE32, "type" },
    { Primitive::TYPE32, "_padding" },
    { Primitive::UINT64, "id" },
    { Primitive::UINT64, "teamId" },
    { Primitive::UINT32, "sortOrder" },
    { Primitive::STRING, "title" },
    { },
};

Schema schemas[] = {
    "team", teamFields,
    "item", itemFields,
};

enum struct Type : uint32_t {
    TEAM = 0,
    ITEM = 1,
};

void print(uint8_t *record) {
    auto type = getInt<uint32_t>(record, 0);
    auto id = getInt<uint32_t>(record, 8);

    size_t offset = 0;

    for (Field *field = schemas[type].fields; field->type != Primitive::NVALID; ++field) {
        switch (field->type) {
            case Primitive::UINT32:
                cout << format(field->name, getInt<uint32_t>(record, offset)) << endl;
                break;
            case Primitive::UINT64:
                cout << format(field->name, getInt<uint64_t>(record, offset)) << endl;
                break;
            case Primitive::STRING:
                auto string = getString(record, offset);
                cout << format(field->name, string.data) << endl;
                offset += (string.length + 8 - 1) / 8 * 8;
                break;
        }

        offset += primitiveSizes[static_cast<size_t>(field->type)];
    }
}

//

template<int TLength> struct _String {
    uint32_t length;
    char title[TLength];
};

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
    uint32_t sortOrder;
    _String<TTitleLength> title;
};

struct Data {
    Team team1 = { Type::TEAM, 0, 100 };
    Team team2 = { Type::TEAM, 0, 101 };
    Item<4> item1 = { Type::ITEM, 0, 2000, 100, 3, { 4, { 'A', 'B', 'C', 0 } } };
    Item<12> item2 = { Type::ITEM, 0, 2001, 100, 2, { 12, { 'D', 'E', 'F', 0 } } };
    Item<20> item3 = { Type::ITEM, 0, 2002, 101, 1, { 20, { 'G', 'H', 'I', 0 } } };
    Item<28> item4 = { Type::ITEM, 0, 2003, 101, 0, { 28, { 'J', 'K', 'L', 0 } } };
} data;

struct Teams {
    Team team1 = { Type::TEAM, 0, 100 };
    Team team2 = { Type::TEAM, 0, 101 };
} teams;

struct Items {
    Item<4> item1 = { Type::ITEM, 0, 2000, 100, 3, { 4, { 'A', 'B', 'C', 0 } } };
    Item<12> item2 = { Type::ITEM, 0, 2001, 100, 2, { 12, { 'D', 'E', 'F', 0 } } };
    Item<20> item3 = { Type::ITEM, 0, 2002, 101, 1, { 20, { 'G', 'H', 'I', 0 } } };
    Item<28> item4 = { Type::ITEM, 0, 2003, 101, 0, { 28, { 'J', 'K', 'L', 0 } } };
} items;

//

uint64_t itemIdIndex[] = {
    2000, 2001, 2002, 2003,
};

uint64_t itemIdIndexData[] = {
    0, 40, 88, 144,
};

//

uint64_t itemTeamIdIndex[] = {
    100, 101,
};

uint16_t itemTeamIdIndexIndex[] = {
    0, 2,
};

size_t itemTeamIdIndexData[] = {
    40, 0, 144, 88,
};

uint8_t *findItemWithId(uint64_t id) {
    auto index = lowerBound(itemIdIndex, sizeof(itemIdIndex), id);

    if (index) {
        uint8_t *ptr = reinterpret_cast<uint8_t *>(&items) + itemIdIndexData[*index];

        return ptr;
    }

    cerr << "Item not found with id " << id << endl;

    return 0;
}

int main() {
    auto item = findItemWithId(2000);

    print(item);

    cout << endl << "TEAMS" << endl << endl;

    cout << format("Offset", "ID") << endl;
    cout << repeat("===============", 2) << endl;

    auto ptr = reinterpret_cast<uint8_t *>(&teams);

    while (ptr < reinterpret_cast<uint8_t *>(&teams) + sizeof(teams)) {
        auto type = getInt<uint32_t>(ptr, 0);
        auto id = getInt<uint64_t>(ptr, 8);

        cout << format(ptr - reinterpret_cast<uint8_t *>(&teams), id) << endl;

        ptr += sizeof(Team);
    }

    cout << endl << "ITEMS" << endl << endl;

    cout << format("Offset", "ID", "Team ID", "Length", "Title") << endl;
    cout << repeat("===============", 5) << endl;

    ptr = reinterpret_cast<uint8_t *>(&items);

    while (ptr < reinterpret_cast<uint8_t *>(&items) + sizeof(items)) {
        auto type = getInt<uint32_t>(ptr, 0);
        auto id = getInt<uint64_t>(ptr, 8);
        auto teamId = getInt<uint64_t>(ptr, 16);
        auto title = getString(ptr, 28);

        cout << format(ptr - reinterpret_cast<uint8_t *>(&items), id, teamId, title.length, title.data) << endl;

        ptr += sizeof(Item<0>) + (title.length + 8 - 1) / 8 * 8;
    }

    cout << endl << "ITEMS WITH TEAM_ID = 100 SORTED BY SORT_ORDER" << endl << endl;

    cout << format("Offset", "ID", "Team ID", "Length", "Title") << endl;
    cout << repeat("===============", 5) << endl;

    auto index3 = lowerBound(itemTeamIdIndex, sizeof(itemTeamIdIndex), (uint64_t) 100);

    if (index3) {
        for (size_t index = itemTeamIdIndexIndex[*index3]; index < sizeof(itemTeamIdIndexData) ; ++index) {
            ptr = &reinterpret_cast<uint8_t *>(&items)[itemTeamIdIndexData[index]];

            auto type = getInt<uint32_t>(ptr, 0);
            auto id = getInt<uint64_t>(ptr, 8);
            auto teamId = getInt<uint64_t>(ptr, 16);
            auto title = getString(ptr, 28);

            if (teamId != 100) {
                break;
            }

            cout << format(ptr - reinterpret_cast<uint8_t *>(&items), id, teamId, title.length, title.data) << endl;
        };
    }

    return 0;
}

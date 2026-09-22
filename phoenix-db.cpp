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
    { Primitive::UINT32, "type" },
    { Primitive::UINT32, "_padding" },
    { Primitive::UINT64, "id" },
    { },
};

Field itemFields[] = {
    { Primitive::UINT32, "type" },
    { Primitive::UINT32, "_padding" },
    { Primitive::UINT64, "id" },
    { Primitive::UINT64, "teamId" },
    { Primitive::UINT32, "sortOrder" },
    { Primitive::STRING, "title" },
    { },
};

Table tables[] = {
    "teams", teamFields,
    "items", itemFields,
};

enum struct Type : uint32_t {
    TEAM = 0,
    ITEM = 1,
};

void print(uint8_t *record) {
    auto type = getInt<uint32_t>(record, 0);
    auto id = getInt<uint32_t>(record, 8);

    size_t offset = 0;

    for (Field *field = tables[type].fields; field->type != Primitive::NVALID; ++field) {
        switch (field->type) {
            case Primitive::UINT32:
                cout << std::left << std::setw(16) << field->name << getInt<uint32_t>(record, offset) << endl;
                break;
            case Primitive::UINT64:
                cout << std::left << std::setw(16) << field->name << getInt<uint64_t>(record, offset) << endl;
                break;
            case Primitive::STRING:
                auto string = getString(record, offset);
                cout << std::left << std::setw(16) << field->name << string.data << endl;
                offset += (string.length + 8 - 1) / 8 * 8;
                break;
        }

        offset += field->type == Primitive::STRING ? 4 : static_cast<size_t>(field->type);
    }
}

//

template<int TLength> struct _String {
    uint32_t length;
    char8_t title[TLength];
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
    Team team1 = { Type::TEAM, 0, 1000 };
    Team team2 = { Type::TEAM, 0, 1001 };
    Item<4> item1 = { Type::ITEM, 0, 20000, 1000, 0, { 4, { 'A', 'B', 'C', 0 } } };
    Item<12> item2 = { Type::ITEM, 0, 20001, 1000, 1, { 12, 'D', 'E', 'F', 'G', 'H', 0 } };
    Item<16> item3 = { Type::ITEM, 0, 20002, 1001, 2, { 16, 'I', 'J', 'K', 'L', 0 } };
    Item<10> item4 = { Type::ITEM, 0, 20003, 1001, 3, { 10, 'M', 'N', 'O', 0 } };
} data;

//

uint64_t itemIdIndex[] = {
    20000, 20001, 20002, 20003,
};

uint64_t itemIdIndexData[] = {
    32, 72, 120, 168,
};

//

uint64_t itemTeamIdIndex[] = {
    1000, 1001,
};

uint16_t itemTeamIdIndexIndex[] = {
    0, 2,
};

// Sorted by teamId, sortOrder
size_t itemTeamIdIndexData[] = {
    32, 72, 120, 168,
};

uint8_t *findItemWithId(uint64_t id) {
    auto index = lowerBound(itemIdIndex, sizeof(itemIdIndex), id);

    if (index) {
        uint8_t *ptr = reinterpret_cast<uint8_t *>(&data) + itemIdIndexData[*index];

        return ptr;
    }

    cerr << "Item not found with id " << id << endl;

    return 0;
}

int main() {
    auto item = findItemWithId(20000);

    print(item);

    cout << endl;

    cout << format("Offset", "Type", "ID", "Team ID", "Length", "Title") << endl;
    cout << format("=======", "=======", "=======", "=======", "=======", "=======") << endl;

    uint8_t *ptr = reinterpret_cast<uint8_t *>(&data);

    while (ptr < reinterpret_cast<uint8_t *>(&data) + sizeof(data)) {
        auto type = getInt<uint32_t>(ptr, 0);
        auto id = getInt<uint64_t>(ptr, 8);

        if (static_cast<Type>(type) == Type::TEAM) {
            cout << ptr - reinterpret_cast<uint8_t *>(&data) << "\t" << type << "\t" << id << endl;

            ptr += sizeof(Team);
        } else if (static_cast<Type>(type) == Type::ITEM) {
            auto teamId = getInt<uint64_t>(ptr, 16);
            auto title = getString(ptr, 28);

            cout << ptr - reinterpret_cast<uint8_t *>(&data) << "\t" << format(type, id, teamId, title.length, title.data) << endl;

            ptr += sizeof(Item<0>) + (title.length + 8 - 1) / 8 * 8;
        } else {
            ptr = 0;
        }
    }

    cout << endl;

    auto index = lowerBound(itemIdIndex, sizeof(itemIdIndex), (uint64_t) 20001);

    if (index) {
        uint8_t *ptr = reinterpret_cast<uint8_t *>(&data) + itemIdIndexData[*index];

        cout << getInt<uint32_t>(ptr, 0) << "\t" << getInt<uint64_t>(ptr, 8) << endl;
    } else {
        cerr << "Item not found with id " << 20001 << endl;
    }

    //

    auto index2 = lowerBound(itemTeamIdIndex, sizeof(itemTeamIdIndex), (uint64_t) 1000);

    if (index2) {
      cout << "Found item with teamId " << 1000 << " at index " << *index2 << endl;
    } else {
      cerr << "Item not found with teamId " << 1000 << endl;
    }

    if (index2) {
        ptr = reinterpret_cast<uint8_t *>(&data) + itemTeamIdIndexData[itemTeamIdIndexIndex[*index2]];

        while (ptr < reinterpret_cast<uint8_t *>(&data) + sizeof(data)) {
            auto type = getInt<uint32_t>(ptr, 0);
            auto id = getInt<uint64_t>(ptr, 8);
            auto teamId = getInt<uint64_t>(ptr, 16);
            auto title = getString(ptr, 28);

            if (teamId != 1000) {
                break;
            }

            cout << ptr - reinterpret_cast<uint8_t *>(&data) << "\t" << format(type, id, teamId, title.length, title.data) << endl;

            ptr += sizeof(Item<0>) + (title.length + 8 - 1) / 8 * 8;
          }
    }

    // benchmark();

    return 0;
}

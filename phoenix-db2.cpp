// g++ -std=c++20 -O3 -flto phoenix-db.cpp

#include <iostream>
#include <cstdint>
#include <vector>
#include <map>

using std::string, std::vector, std::map;
using std::cout, std::endl;

enum struct Type : u_int32_t {
    TYPE32 = 0 | 4 << 4,
    UINT32 = 1 | 4 << 4,
    UINT64 = 2 | 8 << 4,
    STRING = 3 | 8 << 4,
    TARRAY = 4 | 0 << 4,
    OBJECT = 5 | 8 << 4,
};

struct Column;

struct UniqueIndex {
    const vector<uint64_t> ids;
    const vector<size_t> offsets;
};

struct Table {
    const string name;
    const vector<Column> columns;
    const UniqueIndex primaryIndex;
} nullTable;

struct Column {
    const string name;
    const Type type;
    const Table& table = nullTable;
};

extern const Table teamsTable;
extern const Table itemsTable;

const Table teamsTable = {
    "teams",
    {
        { "id", Type::UINT64 },
        { "name", Type::STRING },
        { "items", Type::TARRAY, itemsTable },
    },
};

const Table itemsTable = {
    "items",
    {
        { "id", Type::UINT64 },
        { "name", Type::STRING },
        { "team", Type::OBJECT, teamsTable },
    },
    {
        { 2000, 2001, 2002, 2003 },
        { 0, 24, 48, 72 },
    },
};

map<string, const Table&> tables = {
    { "teams", teamsTable },
    { "items", itemsTable },
};

//

struct Team {
    const uint64_t id;
    const uint64_t name;
};

struct Item {
    const uint64_t id;
    const uint64_t team_id;
    const uint64_t title;
};

//

int main() {
    cout << sizeof(Item) << endl;

    struct Teams {
        Team team1 = { 100, 0x000031206d616554 };
        Team team2 = { 101, 0x000032206d616554  };
    } teams;
    
    struct Items {
        Item item1 = { 2000, 100, 0x000031206d657449 };
        Item item2 = { 2001, 100, 0x000032206d657449 };
        Item item3 = { 2002, 101, 0x000033206d657449 };
        Item item4 = { 2003, 101, 0x000034206d657449 };
    } items;

    for (size_t i = 0; auto id : itemsTable.primaryIndex.ids) {
        auto xxx = itemsTable.primaryIndex.offsets[i++];

        auto data = reinterpret_cast<uint8_t *>(&items);

        cout << id << "\t" << xxx << "\t" << *reinterpret_cast<uint64_t *>(data + xxx) << endl;
    }

    return 0;
}

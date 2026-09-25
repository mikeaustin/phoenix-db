// g++ -std=c++20 -O3 -flto phoenix-db.cpp

#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "table-utils.h"
#include "file-utils.h"

using std::cout;
using std::cerr;
using std::endl;

extern Table teamsTable;
extern Table itemsTable;

Table teamsTable = {
    "teams",
    new Column[] {
        { Primitive::UINT64, "id" },
        { },
    },
    new Relationship[] {
        { "items", "id", &itemsTable, Relationship::ONE_TO_MANY },
    },
};

Table itemsTable = {
    "items", 
    new Column[] {
        { Primitive::UINT64, "id" },
        { Primitive::UINT64, "teamId" },
        { Primitive::UINT32, "sortOrder" },
        { Primitive::STRING, "title" },
        { },
    },
    new Relationship[] {
        { "team", "teamId", &teamsTable, Relationship::ONE_TO_MANY },
    },
};

Table tables[] = {
    teamsTable,
    itemsTable,
};

//

template<int TLength> struct _String {
    const uint32_t length;
    char title[TLength];
};

struct Team {
    const uint64_t id;
};

template<int TTitleLength> struct Item {
    const uint64_t id;
    const uint64_t teamId;
    const uint32_t sortOrder;
    const _String<TTitleLength> title;
};

struct Teams {
    Team team1 = { 100 };
    Team team2 = { 101 };
} teams;

struct Items {
    Item<4> item1 = { 2000, 100, 3, { 4, { 'A', 'B', 'C', 0 } } };
    Item<12> item2 = { 2001, 100, 2, { 12, { 'D', 'E', 'F', 0 } } };
    Item<20> item3 = { 2002, 101, 1, { 20, { 'G', 'H', 'I', 0 } } };
    Item<28> item4 = { 2003, 101, 0, { 28, { 'J', 'K', 'L', 0 } } };
} _items;

//

Index<uint64_t, 4> itemIdIndex = {
    { 2000, 2001, 2002, 2003 },
    { 0, 32, 72, 120 },
};

Index2<uint64_t> itemIdIndex2[] = {
    { 2000, 0 }, { 2001, 32 }, { 2002, 72 }, { 2003, 120 },
};

IndexIndex<uint64_t, 3, 4> itemTeamIdIndex = {
    { 100, 101 },
    { 0, 2, 4 },
    { 32, 0, 120, 72 },
};

std::optional<Record> findItemWithId(uint8_t *items, uint64_t id) {
    auto index = lowerBound(itemIdIndex.ids, sizeof(itemIdIndex.ids), id);

    if (index) {
        auto ptr = getRecord(items, itemIdIndex.offsets[*index]);

        return Record { itemsTable.columns, ptr };
    }

    cerr << "Item not found with id " << id << endl;

    return std::nullopt;
}

//
//
//

int main() {
    auto xxx = lowerBound2(itemIdIndex2, sizeof(itemIdIndex2), (uint64_t) 2001);

    if (xxx) {
        cout << xxx->id << endl;
    }

    uint8_t *items = openTable("example.bin");

    std::memcpy(items, &_items, sizeof(_items));

    {
        cout << "TEAMS" << endl << endl;

        cout << format("Offset", "ID") << endl;
        cout << repeat("===============", 2) << endl;

        auto first = getRecord(&teams, 0),
             last = getRecord(&teams, sizeof(teams)),
             record = first;

        while (record < last) {
            auto id = getInt<uint64_t>(record, 0);

            printRow(record - first, { teamsTable.columns, record });

            record += recordSize({ teamsTable.columns, record });
        }
    }

    {
        cout << endl << "ITEMS" << endl << endl;

        cout << format("Offset", "ID", "Team ID", "Sort Order", "Title") << endl;
        cout << repeat("===============", 5) << endl;

        auto first = getRecord(items, 0),
             last = getRecord(items, sizeof(_items)),
             record = first;

        while (record < last) {
            auto title = getString(record, 20);

            printRow(record - first, { itemsTable.columns, record });

            record += recordSize({ itemsTable.columns, record });
        }
    }

    {
        cout << endl << "ITEM WHERE ID = 2000" << endl << endl;

        auto item = findItemWithId(reinterpret_cast<uint8_t *>(items), 2000);

        if (item) {
            print(*item);
        }
    }

    {
        cout << endl << "ITEMS WHERE TEAM_ID = 100 SORTED BY SORT_ORDER" << endl << endl;

        cout << format("Offset", "ID", "Team ID", "Sort Order", "Title") << endl;
        cout << repeat("===============", 5) << endl;

        auto teamIndex = lowerBound(itemTeamIdIndex.ids, sizeof(itemTeamIdIndex.ids), (uint64_t) 100);

        if (teamIndex) {
            for (size_t offsetIndex = itemTeamIdIndex.indexes[*teamIndex]; offsetIndex < sizeof(itemTeamIdIndex.offsets) / sizeof(size_t) ; ++offsetIndex) {
                auto record = getRecord(items, itemTeamIdIndex.offsets[offsetIndex]);

                if (offsetIndex >= itemTeamIdIndex.indexes[*teamIndex + 1]) {
                    break;
                }

                printRow(record - reinterpret_cast<uint8_t *>(items), { itemsTable.columns, record });
            };
        }
    }

    return 0;
}

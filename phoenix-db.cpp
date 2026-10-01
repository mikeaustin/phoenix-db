// g++ -std=c++20 -O3 -flto phoenix-db.cpp

#include <iostream>
#include <map>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils/table-utils.h"
#include "utils/file-utils.h"
#include "utils/search-utils.h"
#include "utils/query-utils.h"
#include "utils/print-utils.h"
#include "utils/network-utils.h"

using std::cin, std::cout;

extern Table teamsTable;
extern Table itemsTable;

Table teamsTable = {
    "teams",
    {
        { "id", Primitive::UINT64 },
        { "padding", Primitive::UINT32 },
        { "name", Primitive::STRING },
    },
    {
        { "items", &itemsTable, "id", Relationship::ONE_TO_MANY },
    },
};

Table itemsTable = {
    "items", 
    {
        { "id", Primitive::UINT64 },
        { "team_id", Primitive::UINT64 },
        { "sort_order", Primitive::UINT32 },
        { "title", Primitive::STRING },
    },
    {
        { "team", &teamsTable, "team_id", Relationship::ONE_TO_ONE },
    },
    {
        { 2000, 2001, 2002, 2003 },
        { 0, 32, 72, 120 },
    }
};

std::map<string, Table *> tables = {
    { "teams", &teamsTable },
    { "items", &itemsTable },
};

//

template<int TLength> struct _String {
    const uint32_t length;
    char title[TLength];
};

template<int TNameLength>
struct Team {
    const uint64_t id;
    const uint32_t sort_order;
    const _String<TNameLength> name;
};

template<int TTitleLength>
struct Item {
    const uint64_t id;
    const uint64_t team_id;
    const uint32_t sort_order;
    const _String<TTitleLength> title;
};

struct Teams {
    Team<10> team1 = { 100, 1, { 10, { 'T', 'e', 'a', 'm', ' ', 'A', 0 } } };
    Team<10> team2 = { 101, 2, { 10, { 'T', 'e', 'a', 'm', ' ', 'B', 0 } } };
} _teams;

struct Items {
    Item<4> item1 = { 2000, 100, 3, { 4, { 'A', 'B', 'C', 0 } } };
    Item<12> item2 = { 2001, 100, 2, { 12, { 'D', 'E', 'F', 0 } } };
    Item<20> item3 = { 2002, 101, 1, { 20, { 'G', 'H', 'I', 0 } } };
    Item<28> item4 = { 2003, 101, 0, { 28, { 'J', 'K', 'L', 0 } } };
} _items;

//

Index2<uint64_t> itemIdIndex2[] = {
    { 2000, 0 }, { 2001, 32 }, { 2002, 72 }, { 2003, 120 },
};

NonUniqueIndex<uint64_t, 3, 4> itemTeamIdIndex = {
    { 100, 101 },
    { 0, 2, 4 },
    { 32, 0, 120, 72 },
};

int main(int argc, char *argv[]) {
    teamsTable.data = reinterpret_cast<uint8_t *>(&_teams);
    teamsTable.size = sizeof(_teams);
    itemsTable.data = openTable("items.table");
    itemsTable.size = sizeof(_items);

    std::memcpy(itemsTable.data, &_items, sizeof(_items));

    //

    auto xxx = lowerBound2(itemIdIndex2, sizeof(itemIdIndex2) / sizeof(uint64_t), (uint64_t) 2001);

    if (xxx) {
        cout << xxx->id << endl;
    }

    cout << endl;
    
    dumpTable(&teamsTable, {}, "TEAMS");
    dumpTable(&itemsTable, {}, "\n\nITEMS");

    {
        printHeader(itemsTable, {}, "\n\nITEM WHERE ID = 2000");

        auto index = lowerBound(itemsTable.primaryIndex.ids.data(), itemsTable.primaryIndex.ids.size(), (uint64_t) 2000);

        if (index) {
            auto row = getRow(itemsTable.data, itemsTable.primaryIndex.offsets[*index]);

            printRecord({ &itemsTable, row });
        } else {
            cerr << "Item not found with id " << 2000 << endl;
        }
    }

    {
        printHeader(itemsTable, {}, "\n\nITEMS WHERE TEAM_ID = 100 SORTED BY SORT_ORDER");

        auto teamIndex = lowerBound(itemTeamIdIndex.ids, sizeof(itemTeamIdIndex.ids) / sizeof(uint64_t), (uint64_t) 100);

        if (teamIndex) {
            dumpTableByNonUniqueIndex(&itemsTable, itemTeamIdIndex, *teamIndex);
        }
    }

    cout << endl;

    //

    // startServer();

    //

    string query;

    while (true) {
        cout << "] "; std::getline(cin, query);
        auto begin = query.begin(), end = query.end();

        auto expr = expression(begin, end);
        auto table = tables.find(expr.table.value);

        if (table != tables.end()) {
            dumpTable(table->second, expr.fields);
        }
    }

    return 0;
}

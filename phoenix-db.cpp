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
        { "id", Type::UINT64 },
        { "name", Type::STRING },
    },
    {
        { "items", &itemsTable, "id" },
    },
};

Table itemsTable = {
    "items", 
    {
        { "id", Type::UINT64 },
        { "team_id", Type::UINT64, &teamsTable },
        { "sort_order", Type::UINT32 },
        { "padding", Type::UINT32 },
        { "title", Type::STRING },
    },
    {
        { "team", &teamsTable, "id", "team_id" },
    },
    {
        "pkey",
        { 2000, 2001, 2002, 2003 },
        { 0, 40, 88, 144 },
    },
    {
        {
            "teamId", 
            { 100, 101 },
            { 0, 2, 4 },
            { 40, 0, 144, 88 },
        }
    }
};

std::map<string, Table *> tables = {
    { "teams", &teamsTable },
    { "items", &itemsTable },
};

//

template<int TLength> struct _String {
    const uint32_t length;
    const uint32_t padding;
    char title[TLength];
};

template<int TNameLength>
struct Team {
    const uint64_t id;
    const _String<TNameLength> name;
};

template<int TTitleLength>
struct Item {
    const uint64_t id;
    const uint64_t team_id;
    const uint32_t sort_order;
    const uint32_t padding;
    const _String<TTitleLength> title;
};

struct Teams {
    Team<10> team1 = { 100, { 10, 0, { 'T', 'e', 'a', 'm', ' ', 'A', 0 } } };
    Team<10> team2 = { 101, { 10, 0, { 'T', 'e', 'a', 'm', ' ', 'B', 0 } } };
} _teams;

struct Items {
    Item<4> item1 = { 2000, 100, 3, 0, { 4, 0, { 'A', 'B', 'C', 0 } } };
    Item<12> item2 = { 2001, 100, 2, 0, { 12, 0, { 'D', 'E', 'F', 0 } } };
    Item<20> item3 = { 2002, 101, 1, 0, { 20, 0, { 'G', 'H', 'I', 0 } } };
    Item<28> item4 = { 2003, 101, 0, 0, { 28, 0, { 'J', 'K', 'L', 0 } } };
} _items;

//

bool is_newer(uint8_t i1, uint8_t i2) {
    return (int8_t) (i1 - i2) > 0;
}

int main(int argc, char *argv[]) {
    cout << is_newer(255, 254) << endl;
    cout << is_newer(126, 255) << endl;
    cout << is_newer(127, 255) << endl;
    cout << is_newer(128, 255) << endl;

    teamsTable.data = reinterpret_cast<uint8_t *>(&_teams);
    teamsTable.size = sizeof(_teams);
    itemsTable.data = openTable("items.table");
    itemsTable.size = sizeof(_items);

    std::memcpy(itemsTable.data, &_items, sizeof(_items));

    //
    
    dumpTable(&teamsTable, {}, "TEAMS");
    dumpTable(&itemsTable, {}, "\n\nITEMS");

    {
        printHeader(itemsTable, {}, "\n\nITEM WHERE ID = 2000");

        if (auto index = lowerBound(itemsTable.primaryIndex.ids.data(), itemsTable.primaryIndex.ids.size(), (uint64_t) 2000)) {
            auto row = getRow(itemsTable.data, itemsTable.primaryIndex.offsets[*index]);

            printRecord({ &itemsTable, row });
        } else {
            cerr << "Item not found with id " << 2000 << endl;
        }
    }

    {
        printHeader(itemsTable, {}, "\n\nITEMS WHERE TEAM_ID = 100 SORTED BY SORT_ORDER");

        if (auto teamIndex = lowerBound(itemsTable.secondaryIndexes[0].ids.data(), itemsTable.secondaryIndexes[0].ids.size(), (uint64_t) 100)) {
            dumpTableByNonUniqueIndex(&itemsTable, itemsTable.secondaryIndexes[0], *teamIndex);
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
        } else {
            cout << "Table '" << expr.table.value << "' not found" << endl;
        }
    }

    return 0;
}

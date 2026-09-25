// g++ -std=c++20 -O3 -flto phoenix-db.cpp

#include <iostream>
#include <map>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "table-utils.h"
#include "file-utils.h"
#include "print-utils.h"
#include "search-utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

extern Table teamsTable;
extern Table itemsTable;

Table teamsTable = {
    "teams",
    {
        { "id", Primitive::UINT64 },
    },
    new Relationship[] {
        { "items", &itemsTable, "id", Relationship::ONE_TO_MANY },
    },
};

Table itemsTable = {
    "items", 
    {
        { "id", Primitive::UINT64 },
        { "teamId", Primitive::UINT64 },
        { "sortOrder", Primitive::UINT32 },
        { "title", Primitive::STRING },
    },
    new Relationship[] {
        { "team", &teamsTable, "teamId", Relationship::ONE_TO_ONE },
    },
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

        return Record { &itemsTable, ptr };
    }

    cerr << "Item not found with id " << id << endl;

    return std::nullopt;
}

//
//
//

struct Keyword {
    enum Value : int8_t {
        SELECT = 'S',
    };

    Value value;
};

struct Identifier {
    string value;
};

struct Statement {
    Keyword keyword;
    Identifier table;
};

void spaces(string::iterator& input, string::iterator end) {
    while (input != end && *input == ' ') {
        ++input;
    }
}

std::optional<Keyword> keyword(string::iterator& input, string::iterator end) {
    string keyword(input, input + 6);

    if (keyword == "select") {
        input += 6;

        return Keyword { Keyword::SELECT };
    }

    return std::nullopt;
}

std::optional<Identifier> identifier(string::iterator& input, string::iterator end) {
    auto it = input;

    int i = 0;

    while (it != end && *it != ' ') {
        ++it;
        ++i;
    }

    string identifier(input, it);

    input += i;

    return Identifier { identifier };
}

Statement expression(string::iterator& input, string::iterator end) {
    auto _keyword = keyword(input, end);
    spaces(input, end);
    auto _table = identifier(input, end);

    if (_table) {
        return Statement { *_keyword, *_table };
    }

    return Statement { };
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: a.out query" << endl;

        exit(1);
    }

    teamsTable.data = reinterpret_cast<uint8_t *>(&teams);
    teamsTable.size = sizeof(teams);
    itemsTable.data = openTable("example.bin");
    itemsTable.size = sizeof(_items);

    std::memcpy(itemsTable.data, &_items, sizeof(_items));

    //

    auto query = std::string(argv[1]);

    auto begin = query.begin(), end = query.end();

    auto expr = expression(begin, end);

    dump(*tables.at(expr.table.value));

    //

    auto xxx = lowerBound2(itemIdIndex2, sizeof(itemIdIndex2), (uint64_t) 2001);

    if (xxx) {
        cout << xxx->id << endl;
    }

    cout << endl; dump(teamsTable); cout << endl; dump(itemsTable);

    {
        cout << endl << "ITEM WHERE ID = 2001" << endl << endl;

        auto item = findItemWithId(reinterpret_cast<uint8_t *>(itemsTable.data), 2001);

        if (item) {
            printRow(item->data - itemsTable.data, *item);
        }
    }

    {
        cout << endl << "ITEMS WHERE TEAM_ID = 100 SORTED BY SORT_ORDER" << endl << endl;

        cout << format("Offset", "ID", "Team ID", "Sort Order", "Title") << endl;
        cout << repeat("===============", 5) << endl;

        auto teamIndex = lowerBound(itemTeamIdIndex.ids, sizeof(itemTeamIdIndex.ids), (uint64_t) 100);

        if (teamIndex) {
            for (size_t offsetIndex = itemTeamIdIndex.indexes[*teamIndex]; offsetIndex < sizeof(itemTeamIdIndex.offsets) / sizeof(size_t) ; ++offsetIndex) {
                auto record = getRecord(itemsTable.data, itemTeamIdIndex.offsets[offsetIndex]);

                if (offsetIndex >= itemTeamIdIndex.indexes[*teamIndex + 1]) {
                    break;
                }

                printRow(record - reinterpret_cast<uint8_t *>(itemsTable.data), { &itemsTable, record });
            };
        }
    }

    return 0;
}

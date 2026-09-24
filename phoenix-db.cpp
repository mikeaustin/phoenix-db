// g++ -std=c++20 -O3 -flto phoenix-db.cpp

#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;

template <size_t S>
struct Relation {
    const char *name;
    const char *foreignKey;
    const Schema<S>& relation;
};

const Schema teamSchema = {
    "team",
    {
        { Primitive::UINT64, "id" },
        { },
    },
};

const Schema itemSchema = {
    "team",
    {
        { Primitive::UINT64, "id" },
        { Primitive::UINT64, "teamId" },
        { Primitive::UINT32, "sortOrder" },
        { Primitive::STRING, "title" },
        { },
    },
};

Relation<5> teamRelations[] = {
    { "items", "teamId", itemSchema }
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
} items;

//

uint64_t itemIdIndex[] = {
    2000, 2001, 2002, 2003,
};

size_t itemIdIndexData[] = {
    0, 32, 72, 120,
};

uint64_t itemTeamIdIndex[] = {
    100, 101,
};

uint16_t itemTeamIdIndexIndex[] = {
    0, 2,
};

size_t itemTeamIdIndexIndexData[] = {
    32, 0, 120, 72,
};

std::optional<Record> findItemWithId(uint8_t *items, uint64_t id) {
    auto index = lowerBound(itemIdIndex, sizeof(itemIdIndex), id);

    if (index) {
        auto ptr = getRecord(items, itemIdIndexData[*index]);

        return Record { itemSchema.fields, ptr };
    }

    cerr << "Item not found with id " << id << endl;

    return std::nullopt;
}

//
//
//

int main() {
    const char *filename = "example.bin";
    const size_t FILE_SIZE = 4096;

    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        std::perror("Error opening/creating file");

        return 1;
    }

    if (ftruncate(fd, FILE_SIZE) == -1) {
        std::perror("Error setting file size");
        close(fd);

        return 1;
    }

    void *map = mmap(nullptr, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (map == MAP_FAILED) {
        std::perror("Error mapping the file");
        close(fd);

        return 1;
    }

    close(fd);

    uint8_t *data = static_cast<uint8_t *>(map);

    std::memcpy(data, &items, sizeof(items));

    {
        cout << "TEAMS" << endl << endl;

        cout << format("Offset", "ID") << endl;
        cout << repeat("===============", 2) << endl;

        auto first = getRecord(&teams, 0),
             last = getRecord(&teams, sizeof(teams)),
             record = first;

        while (record < last) {
            auto id = getInt<uint64_t>(record, 0);

            cout << format(record - first, id) << endl;

            record += sizeof(Team);
        }
    }

    {
        cout << endl << "ITEMS" << endl << endl;

        cout << format("Offset", "ID", "Team ID", "Sort Order", "Title") << endl;
        cout << repeat("===============", 5) << endl;

        cout << getInt<uint32_t>(reinterpret_cast<uint8_t *>(&items), 0) << endl;
        cout << getInt<uint32_t>(reinterpret_cast<uint8_t *>(data), 0) << endl;

        auto first = getRecord(&items, 0),
             last = getRecord(&items, sizeof(items)),
             record = first;

        while (record < last) {
            auto id = getInt<uint64_t>(record, 0);
            auto teamId = getInt<uint64_t>(record, 8);
            auto sortOrder = getInt<uint32_t>(record, 16);
            auto title = getString(record, 20);

            printRow(record - reinterpret_cast<uint8_t *>(&items), { itemSchema.fields, record });

            record += sizeof(Item<0>) + (title.length + 8 - 1) / 8 * 8;
        }
    }

    {
        cout << endl << "ITEM WHERE ID = 2000" << endl << endl;

        auto item = findItemWithId(reinterpret_cast<uint8_t *>(&items), 2000);

        if (item) {
            print(*item);
        }
    }

    {
        cout << endl << "ITEMS WHERE TEAM_ID = 100 SORTED BY SORT_ORDER" << endl << endl;

        cout << format("Offset", "ID", "Team ID", "Sort Order", "Title") << endl;
        cout << repeat("===============", 5) << endl;

        auto index3 = lowerBound(itemTeamIdIndex, sizeof(itemTeamIdIndex), (uint64_t) 100);

        if (index3) {
            for (size_t index = itemTeamIdIndexIndex[*index3]; index < sizeof(itemTeamIdIndexIndexData) ; ++index) {
                auto record = getRecord(&items, itemTeamIdIndexIndexData[index]);

                auto id = getInt<uint64_t>(record, 0);
                auto teamId = getInt<uint64_t>(record, 8);
                auto sortOrder = getInt<uint32_t>(record, 16);
                auto title = getString(record, 20);

                if (teamId != 100) {
                    break;
                }

                printRow(record - reinterpret_cast<uint8_t *>(&items), { itemSchema.fields, record });
            };
        }
    }

    return 0;
}

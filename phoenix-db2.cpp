// g++ -std=c++20 -O3 -flto phoenix-db.cpp

#include <iostream>
#include <cstdint>
#include <optional>
#include <vector>
#include <map>

using std::string, std::vector, std::map;
using std::cout, std::endl;

enum struct Type : u_int32_t {
    TYPE32 = 0 << 8 | 4,
    UINT32 = 1 << 8 | 4,
    UINT64 = 2 << 8 | 8,
    STRING = 3 << 8 | 8,
    TARRAY = 4 << 8 | 0,
    OBJECT = 5 << 8 | 8,
};

struct Table;

extern Table nullTable;

struct UniqueIndex {
    const vector<uint64_t> ids;
    const vector<size_t> offsets;
};

struct Field {
    const string name;
    const Type type;
    const Table& table = nullTable;
};

struct Table {
    const string name;
    const vector<Field> columns;
    const UniqueIndex primaryIndex;
    uint8_t *data;
} nullTable;

extern Table teamsTable;
extern Table itemsTable;

Table teamsTable = {
    "teams",
    {
        { "id", Type::UINT64 },
        { "name", Type::STRING },
        { "items", Type::TARRAY, itemsTable },
    },
};

Table itemsTable = {
    "items",
    {
        { "id", Type::UINT64 },
        { "team_id", Type::OBJECT, teamsTable },
        { "title", Type::STRING },
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

size_t getFieldOffset(const vector<Field>& fields, const string& name) {
    size_t fieldOffset = 0;

    for (auto field : fields) {
        if (field.name == name) {
            return fieldOffset;
        }

        fieldOffset += static_cast<uint32_t>(field.type) & 0xFF;
    }

    return fieldOffset;
}

struct Record {
    const Table& table;
    const uint8_t *row;
};

const Record getRecord(const Table& table, size_t offset) {
    return { table, &table.data[offset] };
}

const char *getString(const Record& record, const string& name) {
    auto fieldOffset = getFieldOffset(record.table.columns, name);

    return reinterpret_cast<const char *>(&record.row[fieldOffset]);
}

//

template <typename T>
std::optional<size_t> lowerBound(const T *array, size_t count, T value) {
    int left = 0, right = count - 1;

    while (left < right) {
        int mid = left + (right - left) / 2; 

        if (array[mid] >= value) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    if (array[left] == value) {
      return left;
    }

    return std::nullopt;
}

//

int main() {
    // cout << sizeof(Item) << endl;

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

    teamsTable.data = reinterpret_cast<uint8_t *>(&teams);
    itemsTable.data = reinterpret_cast<uint8_t *>(&items);

    //

    for (size_t i = 0; i < itemsTable.primaryIndex.ids.size(); ++i) {
        auto rowOffset = itemsTable.primaryIndex.offsets[i];

        auto record = getRecord(itemsTable, rowOffset);

        cout << i << "\t" << getString(record, "title") << endl;
    }

    if (auto index = lowerBound(itemsTable.primaryIndex.ids.data(), itemsTable.primaryIndex.ids.size(), 2001ULL)) {
        auto record = getRecord(itemsTable, itemsTable.primaryIndex.offsets[*index]);

        cout << getString(record, "title") << endl;
    }

    return 0;
}

// g++ -std=c++20 -O3 -flto phoenix-db.cpp

#include <iostream>
#include <variant>
#include <cstdint>
#include <optional>
#include <chrono>
#include <vector>
#include <map>

using std::string, std::vector, std::map;
using std::cout, std::endl;

enum struct Type : u_int32_t {
    TYPE32 = 0,
    UINT32 = 1,
    UINT64 = 2,
    STRING = 3,
    TARRAY = 4,
    OBJECT = 5,
    DATETS = 6,
};

size_t typeSize[] = {
    4,
    4,
    8,
    8,
    0,
    8,
    8,
};

using Value = std::variant<
    uint32_t,
    uint32_t,
    uint64_t,
    uint64_t,
    void *,
    uint64_t,
    uint64_t
>;

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
        { "created_at", Type::DATETS },
        { "title", Type::STRING },
    },
    {
        { 2000, 2001, 2002, 2003 },
        { 0, 32, 64, 96 },
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
    const uint64_t created_at;
    const uint64_t title;
};

//

size_t getFieldOffset(const vector<Field>& fields, const string& name) {
    size_t fieldOffset = 0;

    for (auto field : fields) {
        if (field.name == name) {
            return fieldOffset;
        }

        fieldOffset += typeSize[static_cast<uint32_t>(field.type)];
    }

    return fieldOffset;
}

size_t getFieldIndex(const vector<Field>& fields, const string& name) {
    size_t fieldIndex = 0;

    for (auto field : fields) {
        if (field.name == name) {
            return fieldIndex;
        }

        fieldIndex += 1;
    }

    return fieldIndex;
}

struct Record {
    const Table& table;
    const uint8_t *row;
};

const Record getRecord(const Table& table, size_t offset) {
    return { table, &table.data[offset] };
}

const int32_t getInt32(const Record& record, const size_t fieldOffset) {
    return *reinterpret_cast<const uint32_t *>(&record.row[fieldOffset]);
}

const int64_t getInt64(const Record& record, const size_t fieldOffset) {
    return *reinterpret_cast<const uint64_t *>(&record.row[fieldOffset]);
}

const char *getString(const Record& record, const string& name) {
    auto fieldOffset = getFieldOffset(record.table.columns, name);

    return reinterpret_cast<const char *>(&record.row[fieldOffset]);
}

const Value getValue(const Record& record, const string& name) {
    auto fieldIndex = getFieldIndex(record.table.columns, name);
    auto fieldOffset = getFieldOffset(record.table.columns, name);

    auto field = record.table.columns.at(fieldIndex);

    switch (static_cast<uint32_t>(field.type)) {
        case 2: return Value { std::in_place_index<2>, getInt32(record, fieldOffset) };
        case 6: return Value { std::in_place_index<6>, getInt64(record, fieldOffset) };
    }

    return Value { };
}

std::ostream& operator <<(std::ostream& stream, const Value& value) {
    switch (static_cast<Type>(value.index())) {
        case Type::TYPE32:
            break;
        case Type::UINT32:
            stream << std::get<static_cast<uint32_t>(Type::UINT32)>(value);
        break;
        case Type::UINT64:
            stream << std::get<static_cast<uint32_t>(Type::UINT64)>(value);
        break;
        case Type::STRING:
            stream << std::get<static_cast<uint64_t>(Type::STRING)>(value);
        break;
        case Type::TARRAY:
            break;
        case Type::OBJECT:
            break;
        case Type::DATETS:
            auto tp = std::chrono::system_clock::from_time_t(std::get<static_cast<uint32_t>(Type::DATETS)>(value));

            stream << std::format("{:%Y-%m-%d %H:%M}", tp);
        break;
    }

    return stream;
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
    cout << "sizeof(Item) = " << sizeof(Item) << endl;

    struct Teams {
        Team team1 = { 100, 0x000031206d616554 };
        Team team2 = { 101, 0x000032206d616554  };
    } teams;
    
    struct Items {
        Item item1 = { 2000, 100, 946684860, 0x000031206d657449 };
        Item item2 = { 2001, 100, 946684920, 0x000032206d657449 };
        Item item3 = { 2002, 101, 946684980, 0x000033206d657449 };
        Item item4 = { 2003, 101, 946685040, 0x000034206d657449 };
    } items;

    teamsTable.data = reinterpret_cast<uint8_t *>(&teams);
    itemsTable.data = reinterpret_cast<uint8_t *>(&items);

    //

    for (size_t i = 0; i < itemsTable.primaryIndex.ids.size(); ++i) {
        auto rowOffset = itemsTable.primaryIndex.offsets[i];

        auto record = getRecord(itemsTable, rowOffset);

        auto createdAt = getValue(record, "created_at");

        cout << i << "\t" << getString(record, "title") << "\t\t" << createdAt << endl;
    }

    if (auto index = lowerBound(itemsTable.primaryIndex.ids.data(), itemsTable.primaryIndex.ids.size(), 2001ULL)) {
        auto record = getRecord(itemsTable, itemsTable.primaryIndex.offsets[*index]);

        cout << getString(record, "title") << endl;
    }

    return 0;
}

// g++ -std=c++20 -O3 -flto -Wimplicit-fallthrough phoenix-db2.cpp

#include <iostream>
#include <variant>
#include <cstdint>
#include <optional>
#include <chrono>
#include <vector>
#include <map>

using std::string, std::vector, std::map;
using std::cout, std::endl;

template <typename T>
struct Array2 {
    void push_back(const T& value) {
        rows.push_back(value);
    }

    const vector<string> fieldNames;
    vector<T> rows;
};

enum struct Id : uint64_t { };
enum struct Offset : size_t { };
enum struct Data : uint8_t { };

size_t typeSize[] = {
    4,
    8,
    8,
    0,
    8,
    8,
};

struct Value;

using Int32 = uint32_t;
using Int64 = uint64_t;
using String = uint64_t;
using Array = Array2<Value>;
using Object = uint64_t;
using Time = uint64_t;

struct Value {
    std::variant<Int32, Int64, String, Array, Object, Time> data;
};

//

struct Table;

extern Table nullTable;

struct UniqueIndex {
    const vector<Id> ids;
    const vector<Offset> offsets;
};

struct Field {
    enum Type : u_int32_t {
        UINT32 = 0,
        UINT64 = 1,
        STRING = 2,
        ARRAY = 3,
        OBJECT = 4,
        TIMESTAMP = 5,
    };

    const string name;
    const Type type;
    const Table& table = nullTable;
};

template <Field::Type T>
constexpr auto type_index() {
    return std::in_place_index<static_cast<size_t>(
        static_cast<std::underlying_type_t<decltype(T)>>(T)
    )>;
}

struct Table {
    const string name;
    const vector<Field> columns;
    const UniqueIndex primaryIndex;
    Data *data;
} nullTable;

extern Table teamsTable;
extern Table itemsTable;

Table teamsTable = {
    "teams",
    {
        { "id", Field::UINT64 },
        { "name", Field::STRING },
        { "items", Field::ARRAY, itemsTable },
    },
};

Table itemsTable = {
    "items",
    {
        { "id", Field::UINT64 },
        { "team_id", Field::OBJECT, teamsTable },
        { "created_at", Field::TIMESTAMP },
        { "title", Field::STRING },
    },
    {
        { Id { 2000 }, Id { 2001 }, Id { 2002 }, Id { 2003 } },
        { Offset { 0 }, Offset { 32 }, Offset { 64 }, Offset { 96 } },
    },
};

map<string, const Table&> tables = {
    { "teams", teamsTable },
    { "items", itemsTable },
};

//

struct Team {
    const Id id;
    const String name;
};

struct Item {
    const Id id;
    const Id team_id;
    const Time created_at;
    const String title;
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
    const Data *row;
};

const Record getRecord(const Table& table, Offset offset) {
    return { table, &reinterpret_cast<Data *>(table.data)[static_cast<size_t>(offset)] };
}

uint32_t getInt32(const Record& record, const size_t fieldOffset) {
    return *reinterpret_cast<const uint32_t *>(&record.row[fieldOffset]);
}

uint64_t getInt64(const Record& record, const size_t fieldOffset) {
    return *reinterpret_cast<const uint64_t *>(&record.row[fieldOffset]);
}

const Value getValue(const Record& record, const string& name) {
    auto fieldIndex = getFieldIndex(record.table.columns, name);
    auto fieldOffset = getFieldOffset(record.table.columns, name);

    auto field = record.table.columns.at(fieldIndex);

    switch (field.type) {
        case Field::UINT32:
            return Value { decltype(Value::data) { type_index<Field::UINT32>(), Int32 { getInt32(record, fieldOffset) } } };
        case Field::UINT64:
            return Value { decltype(Value::data) { type_index<Field::UINT64>(), Int64 { getInt64(record, fieldOffset) } } };
        case Field::STRING:
            return Value { decltype(Value::data) { type_index<Field::STRING>(), String { getInt64(record, fieldOffset) } } };
        case Field::ARRAY:
            return Value { decltype(Value::data) { type_index<Field::ARRAY>(), Array { } } };
        case Field::OBJECT:
            return Value { decltype(Value::data) { type_index<Field::OBJECT>(), Object { getInt64(record, fieldOffset) } } };
        case Field::TIMESTAMP:
            return Value { decltype(Value::data) { type_index<Field::TIMESTAMP>(), Time { getInt64(record, fieldOffset) } } };
    }

    return Value { };
}

std::ostream& operator <<(std::ostream& stream, const Value& value) {
    switch (static_cast<Field::Type>(value.data.index())) {
        case Field::UINT32:
            stream << std::get<static_cast<uint32_t>(Field::UINT32)>(value.data);
            break;
        case Field::UINT64:
            stream << std::get<static_cast<uint32_t>(Field::UINT64)>(value.data);
            break;
        case Field::STRING: {
                auto str = std::get<static_cast<uint32_t>(Field::STRING)>(value.data);

                stream << reinterpret_cast<const char *>(&str);
            }
            break;
        case Field::ARRAY:
            break;
        case Field::OBJECT:
            break;
        case Field::TIMESTAMP: {
                auto tp = std::chrono::system_clock::from_time_t(std::get<static_cast<uint32_t>(Field::TIMESTAMP)>(value.data));

                stream << std::format("{:%Y-%m-%d %H:%M}", tp);
            }
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

struct Results {
    const vector<Field>& fields;
    const Array& rows;
};

void printResults(const Results& results) {
    for (auto row : results.rows.rows) {
        switch(static_cast<Field::Type>(row.data.index())) {
            case Field::OBJECT:
                break;
            case Field::UINT32:
            case Field::UINT64:
            case Field::STRING:
            case Field::TIMESTAMP:
                cout << row << "\t";
                break;
            case Field::ARRAY: {
                auto array = std::get<static_cast<uint32_t>(Field::ARRAY)>(row.data);

                printResults(Results { teamsTable.columns, array });
            }
        }
    }

    cout << endl;
}

//

int main() {
    cout << "sizeof(Item) = " << sizeof(Item) << endl;

    struct Teams {
        Team team1 = { Id { 100 }, String { 0x000031206d616554 } };
        Team team2 = { Id { 101 }, String { 0x000032206d616554 } };
    } teams;
    
    struct Items {
        Item item1 = { Id { 2000 }, Id { 100 }, Time { 946684860 }, String { 0x000031206d657449 } };
        Item item2 = { Id { 2001 }, Id { 100 }, Time { 946684920 }, String { 0x000032206d657449 } };
        Item item3 = { Id { 2002 }, Id { 101 }, Time { 946684980 }, String { 0x000033206d657449 } };
        Item item4 = { Id { 2003 }, Id { 101 }, Time { 946685040 }, String { 0x000034206d657449 } };
    } items;

    teamsTable.data = reinterpret_cast<Data *>(&teams);
    itemsTable.data = reinterpret_cast<Data *>(&items);

    //

    Array rows;
    Array2<Value> rows2 = { { "id" } };

    for (size_t i = 0; i < itemsTable.primaryIndex.ids.size(); ++i) {
        auto rowOffset = itemsTable.primaryIndex.offsets[i];

        auto record = getRecord(itemsTable, rowOffset);

        auto title = getValue(record, "title");
        auto createdAt = getValue(record, "created_at");

        // cout << i << "\t" << title << "\t\t" << createdAt << endl;

        Array columns;

        for (auto field : itemsTable.columns) {
            columns.push_back(getValue(record, field.name));
        }

        rows.push_back(Value { columns });
    }

    auto results = Results { itemsTable.columns, rows };

    printResults(results);

    if (auto index = lowerBound(itemsTable.primaryIndex.ids.data(), itemsTable.primaryIndex.ids.size(), Id { 2001 })) {
        auto record = getRecord(itemsTable, itemsTable.primaryIndex.offsets[*index]);

        cout << getValue(record, "title") << endl;
    }

    return 0;
}

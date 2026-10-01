#ifndef TABLE_UTILS
#define TABLE_UTILS

#include <iostream>
#include <iomanip>
#include <optional>
#include <chrono>

using std::cout, std::cerr, std::endl;
using std::string, std::vector;

namespace Type {
    enum Type : uint32_t {
        TYPE32 = 0,
        UINT32 = 1,
        UINT64 = 2,
        STRING = 3,
    };

    const size_t sizes[] = {
        4,
        4,
        8,
        4,
    };
};

struct Table;

struct Column {
    const string name;
    const Type::Type type;
    const Table *table;
};

//

template <typename T>
struct UniqueIndex {
    const string name;
    const vector<T> ids;
    const vector<size_t> offsets;
};

template <typename T>
struct NonUniqueIndex2 {
    const string name;
    const vector<T> ids;
    const vector<size_t> indexes;
    const vector<size_t> offsets;
};

//

struct Relationship {
    enum Relation : uint32_t {
        ONE_TO_ONE = 0,
        ONE_TO_MANY = 1,
        MANY_TO_MANY = 2,
    };

    const string name;
    const Table *table;
    const string foreignKey;
    const Relation type;
};

struct Table {
    const string name;
    const vector<Column> columns;
    const vector<Relationship> relationships;
    const UniqueIndex<uint64_t> primaryIndex;
    const vector<NonUniqueIndex2<uint64_t>> secondaryIndexes;
    uint8_t *data;
    size_t size;
};

//

struct Record {
    const Table *table;
    uint8_t *data;
};

struct String {
    const uint32_t length;
    const char *data;
};

//

uint8_t *getRow(void *data, size_t offset) {
    return static_cast<uint8_t *>(data) + offset;
}

template <typename T>
T getInt(void *data, size_t offset) {
    if (reinterpret_cast<size_t>(data) % sizeof(T) != 0 || offset % sizeof(T) != 0) {
      throw new std::invalid_argument("Alignment error");
    }

    T value;

    std::memcpy(&value, reinterpret_cast<uint8_t *>(data) + offset, sizeof(T));

    return value;
}

String getString(void *data, size_t offset) {
    auto length = getInt<uint32_t>(data, offset);

    return {
      length,
      reinterpret_cast<char *>(reinterpret_cast<uint8_t *>(data) + 4 + offset)
    };
}

#endif

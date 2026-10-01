#ifndef TABLE_UTILS
#define TABLE_UTILS

#include <iomanip>
#include <optional>
#include <chrono>

using std::cout;
using std::cerr;
using std::endl;

namespace Primitive {
    enum Type : uint32_t {
        TYPE32 = 0,
        UINT32 = 1,
        UINT64 = 2,
        STRING = 3,
    };

    size_t sizes[] = {
        4,
        4,
        8,
        4,
    };
};

struct Column {
    const char *name;
    const Primitive::Type type;
};

//

template <typename T>
struct UniqueIndex {
    std::vector<T> ids;
    std::vector<size_t> offsets;
};

template <typename T>
struct Index2 {
    T id;
    size_t offset;
};

template <typename T, int I, int S>
struct NonUniqueIndex {
    T ids[I];
    size_t indexes[I];
    size_t offsets[S];
};

//

struct Table;

struct Relationship {
    enum Relation : uint32_t {
        ONE_TO_ONE = 0,
        ONE_TO_MANY = 1,
        MANY_TO_MANY = 2,
    };

    const char *name;
    const Table *table;
    const char *foreignKey;
    const Relation type;
};

struct Table {
    const std::string name;
    const std::vector<Column> columns;
    const std::vector<Relationship> relationships;
    const UniqueIndex<uint64_t> primaryIndex;
    uint8_t *data;
    size_t size;
};

//

struct Record {
    const Table *table;
    uint8_t *data;
};

struct String {
    uint32_t length;
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

#include <iomanip>
#include <optional>
#include <chrono>

using std::cout;
using std::cerr;
using std::endl;

namespace Primitive {
    enum Type : uint32_t {
        NVALID = 0,
        TYPE32 = 1,
        UINT32 = 2,
        UINT64 = 3,
        STRING = 4,
    };

    size_t sizes[] = {
        0,
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
    const char *name;
    const Column *columns;
    const Relationship *relationships;
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

template <typename T, int S>
struct Index {
    T ids[S];
    size_t offsets[S];
};

template <typename T>
struct Index2 {
    T id;
    size_t offset;
};

template <typename T, int I, int S>
struct IndexIndex {
    T ids[I];
    size_t indexes[I];
    size_t offsets[S];
};

//

uint8_t *getRecord(void *records, size_t offset) {
    return static_cast<uint8_t *>(records) + offset;
}

template <typename T>
T getInt(uint8_t *data, size_t offset) {
    if (reinterpret_cast<size_t>(data) % sizeof(T) != 0 || offset % sizeof(T) != 0) {
      throw new std::invalid_argument("Alignment error");
    }

    return *reinterpret_cast<T *>(data + offset);
}

String getString(uint8_t *data, size_t offset) {
    auto length = getInt<uint32_t>(data, offset);

    return {
      length,
      reinterpret_cast<char *>(data + 4 + offset)
    };
}

//

template <typename T>
std::optional<size_t> lowerBound(T *array, size_t size, T value) {
    int left = 0, right = size / sizeof(T) - 1;

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

template <typename T, typename U>
T *lowerBound2(T *array, size_t size, U value) {
    int left = 0, right = size / sizeof(T) - 1;

    while (left < right) {
        int mid = left + (right - left) / 2; 

        if (*reinterpret_cast<U *>(&array[mid]) >= value) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    if (*reinterpret_cast<U *>(&array[left]) == value) {
      return &array[left];
    }

    return nullptr;
}

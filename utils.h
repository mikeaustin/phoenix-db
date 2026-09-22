using std::cout;
using std::cerr;
using std::endl;

enum struct Primitive : uint32_t {
    NVALID = 0,
    TYPE32 = 1,
    UINT32 = 2,
    UINT64 = 3,
    STRING = 4,
};

size_t primitiveSizes[] = {
  0,
  4,
  4,
  8,
  4,
};

struct Field {
    Primitive type;
    const char *name;
};

struct Schema {
    const char *name;
    Field *fields;
};

struct String {
  uint32_t length;
  char *data;
};

//

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

//

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct FormatWrapper {
  const T1& a;
  const T2& b;
  const T3& c;
  const T4& d;
  const T5& e;
  const T6& f;
  const T7& g;
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::ostream& operator <<(std::ostream& ostream, const FormatWrapper<T1, T2, T3, T4, T5, T6, T7>& wrapper) {
    ostream << std::left
      << std::setw(16) << wrapper.a
      << std::setw(16) << wrapper.b
      << std::setw(16) << wrapper.c
      << std::setw(16) << wrapper.d
      << std::setw(16) << wrapper.e
      << std::setw(16) << wrapper.f
      << std::setw(16) << wrapper.g;

    return ostream;
}

template <typename T1, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *, typename T5 = const char *, typename T6 = const char *, typename T7 = const char *>
FormatWrapper<T1, T2, T3, T4, T5, T6, T7> format(const T1& a, const T2& b = "", const T3& c = "", const T4& d = "", const T5& e = "", const T6& f = "", const T7& g = "") {
  return FormatWrapper { a, b, c, d, e, f, g };
}

struct repeat {
    const char *string;
    size_t count;

    friend std::ostream& operator <<(std::ostream& ostream, const repeat& repeat) {
        for (size_t i = 0; i < repeat.count; ++i) {
            ostream << std::left << std::setw(16) << repeat.string;
        }

        return ostream;
    }
};

void print(const Schema& schema, uint8_t *record) {
    size_t offset = 0;

    for (Field *field = schema.fields; field->type != Primitive::NVALID; ++field) {
        switch (field->type) {
            case Primitive::UINT32:
                cout << format(field->name, getInt<uint32_t>(record, offset)) << endl;
                break;
            case Primitive::UINT64:
                cout << format(field->name, getInt<uint64_t>(record, offset)) << endl;
                break;
            case Primitive::STRING:
                auto string = getString(record, offset);
                cout << format(field->name, string.data) << endl;
                offset += (string.length + 8 - 1) / 8 * 8;
                break;
        }

        offset += primitiveSizes[static_cast<size_t>(field->type)];
    }
}

enum struct Primitive : uint32_t {
    NVALID = 0,
    UINT32 = 1,
    UINT64 = 2,
    STRING = 3,
};

size_t primitiveSizes[] = {
  0,
  4,
  8,
  4,
};

struct Field {
    Primitive type;
    const char *name;
};

struct Table {
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
std::optional<size_t> lowerBound(T *array, size_t count, T value) {
    int left = 0, right = count / sizeof(T) - 1;

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

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct FormatWrapper {
  const T1& a;
  const T2& b;
  const T3& c;
  const T4& d;
  const T5& e;
  const T6& f;
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::ostream& operator<<(std::ostream& ostream, const FormatWrapper<T1, T2, T3, T4, T5, T6>& wrapper) {
    ostream << wrapper.a << "\t" << wrapper.b << "\t" << wrapper.c << "\t" << wrapper.d << "\t" << wrapper.e << "\t" << wrapper.f;

    return ostream;
}

template <typename T1, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *, typename T5 = const char *, typename T6 = const char *>
FormatWrapper<T1, T2, T3, T4, T5, T6> format(const T1& a, const T2& b = "", const T3& c = "", const T4& d = "", const T5& e = "", const T6& f = "") {
  return FormatWrapper { a, b, c, d, e, f };
}

template<int TLength> struct String {
  int32_t length;
  char8_t title[TLength];
};

enum Type : uint32_t {
  TEAM = 100,
  ITEM = 200,
};

//

template <typename T>
T getInt(int8_t *data, size_t offset) {
  if (reinterpret_cast<size_t>(data) % sizeof(T) != 0 || offset % sizeof(T) != 0) {
    throw new std::invalid_argument("Alignment error");
  }

  return *reinterpret_cast<T *>(data + offset);
}

char *getString(int8_t *data, size_t offset) {
  return reinterpret_cast<char *>(data + offset);
}

//

template <typename TIterator, typename TValue, class TCompare>
TIterator binarySearch2(TIterator low, TIterator high, const TValue& value, TCompare compare) {
  TIterator invalid = high;

  while (low <= high) {
    auto mid = low + (high - low) / 2;
    auto result = compare(*mid, value);

    if (result == 0) {
      return mid;
    } else if (result < 0) {
        low = mid + 1;
    } else {
        high = mid;
    }
  }

  return invalid;
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

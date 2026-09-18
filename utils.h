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

template <typename T1, typename T2, typename T3, typename T4, typename T5>
struct FormatWrapper {
  const T1& a;
  const T2& b;
  const T3& c;
  const T4& d;
  const T5& e;
};

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::ostream& operator<<(std::ostream& os, const FormatWrapper<T1, T2, T3, T4, T5>& wrapper) {
  os << wrapper.a << "\t" << wrapper.b << "\t" << wrapper.c << "\t" << wrapper.d << "\t" << wrapper.e;

  return os;
}

template <typename T1, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *, typename T5 = const char *>
FormatWrapper<T1, T2, T3, T4, T5> format(const T1& a, const T2& b = "", const T3& c = "", const T4& d = "", const T5& e = "") {
  return FormatWrapper { a, b, c, d, e };
}

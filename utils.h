template<int TLength> struct String {
  int32_t length;
  char8_t title[TLength];
};

enum Type : uint32_t {
  TEAM = 100,
  ITEM = 200,
};

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

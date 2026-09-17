// g++ -std=c++20

#include <iostream>
#include <cstdint>

using std::cout;
using std::endl;

//

template<int TLength> struct String {
  int32_t length;
  char8_t title[TLength];
};

enum Type : uint32_t {
  TEAM = 100,
  ITEM = 200,
};

int32_t getInt32(int32_t *data, size_t offset) {
  return *reinterpret_cast<int32_t *>(data + offset);
}

char *getString(int32_t *data, size_t offset) {
  return reinterpret_cast<char *>(data + offset);
}

//

struct Team {
  Type type;
  uint32_t _pad1;
  uint64_t id;
};

template<int TLength> struct Item {
  Type type;
  uint32_t _pad1;
  uint64_t id;
  uint64_t teamId;
  uint32_t _pad2;
  String<TLength> title;
};

struct Data {
  Team team1;
  Team team2;
  Item<4> item1;
  Item<12> item2;
  Item<16> item3;
};

//

int main() {
  Data data = {
    { TEAM, 0, 1000 },
    { TEAM, 0, 1001 },
    { ITEM, 0, 2000, 1000, 0, { 4, { 'A', 'B', 'C', 0 } } },
    { ITEM, 0, 2001, 1000, 0, { 12, 'D', 'E', 'F', 'G', 'H', 0 } },
    { ITEM, 0, 2002, 1000, 0, { 16, 'I', 'J', 'K', 'L', 0 } },
  };

  int32_t *p = reinterpret_cast<int *>(&data);

  while (p != 0) {
    if (*p == Type::TEAM) {
      cout << getInt32(p, 0) << "\t";
      cout << getInt32(p, 2) << endl;

      p += sizeof(Team) / 4;
    } else if (*p == Type::ITEM) {
      auto type = getInt32(p, 0);
      auto id = getInt32(p, 2);
      auto teamId = getInt32(p, 4);
      auto length = getInt32(p, 7);
      auto title = getString(p, 8);

      cout << type << "\t";
      cout << id << "\t";
      cout << teamId << "\t";
      cout << length << "\t";
      cout << title << endl;

      p += (sizeof(Item<0>) + (length + 8 - 1) / 8 * 8) / 4;
    } else {
      p = 0;
    }
  }

  return 0;
}

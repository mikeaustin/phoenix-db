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

int32_t getInt32(int8_t *data, size_t offset) {
  return *reinterpret_cast<int32_t *>(data + offset);
}

char *getString(int8_t *data, size_t offset) {
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

struct TeamIndexData {
  uint64_t teamId;
  uint64_t firstIndex;
};

struct ItemIdIndex {
  uint64_t itemId;
  size_t index;
};

//

int main() {
  Data data = {
    { TEAM, 0, 1000 },
    { TEAM, 0, 1001 },
    { ITEM, 0, 20000, 1000, 0, { 4, { 'A', 'B', 'C', 0 } } },
    { ITEM, 0, 20001, 1000, 0, { 12, 'D', 'E', 'F', 'G', 'H', 0 } },
    { ITEM, 0, 20002, 2000, 0, { 16, 'I', 'J', 'K', 'L', 0 } },
  };

  TeamIndexData indexData[] = {
    { 10000, 0 },
    { 10001, 2 },
  };

  size_t itemTeamEqualityIndex[] = {
    10000, 10001, 10002,
  };

  ItemIdIndex itemIdIndex[] = {
    { 20000, 0 }, { 20001, 1 }, { 20002, 2 },
  };

  cout << "Offset" << "\t" << "Type" << "\t" << "ID" << "\t" << "Team ID" << "\t" << "Length" << "\t" << "Title" << endl;
  cout << "=======" << "\t" << "=======" << "\t" << "=======" << "\t" << "=======" << "\t" << "=======" << "\t" << "=======" << endl;

  int8_t *ptr = reinterpret_cast<int8_t *>(&data);

  while (ptr != 0) {
    auto type = getInt32(ptr, 0);
    auto id = getInt32(ptr, 8);

    if (type == Type::TEAM) {
      cout << ptr - reinterpret_cast<int8_t *>(&data) << "\t" << type << "\t" << id << endl;

      ptr += sizeof(Team);
    } else if (type == Type::ITEM) {
      auto teamId = getInt32(ptr, 16);
      auto length = getInt32(ptr, 28);
      auto title = getString(ptr, 32);

      cout << ptr - reinterpret_cast<int8_t *>(&data) << "\t" << type << "\t" << id << "\t" << teamId << "\t" << length << "\t" << title << endl;

      ptr += sizeof(Item<0>) + (length + 8 - 1) / 8 * 8;
    } else {
      ptr = 0;
    }
  }

  return 0;
}

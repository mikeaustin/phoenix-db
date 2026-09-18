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
  Item<10> item4;
};

struct TeamIndexData {
  uint64_t teamId;
  uint64_t firstIndex;
};

struct ItemIdIndex {
  uint64_t itemId;
  size_t dataIndex;
};

//

int binarySearch(ItemIdIndex array[], size_t size, int target) {
  int low = 0, high = size - 1;

  while (low <= high) {
    int index = low + (high - low) / 2;

    if (array[index].itemId == target) {
        return array[index].dataIndex;
    }
    else if (array[index].itemId < target) {
        low = index + 1;
    }
    else {
        high = index - 1;
    }
  }

  return -1;
}

int binarySearch2(TeamIndexData array[], size_t size, int target) {
  int low = 0, high = size - 1;

  while (low <= high) {
    int index = low + (high - low) / 2;

    if (array[index].teamId == target) {
        return index;
    }
    else if (array[index].teamId < target) {
        low = index + 1;
    }
    else {
        high = index - 1;
    }
  }

  return -1;
}

int main() {
  Data data = {
    { TEAM, 0, 1000 },
    { TEAM, 0, 1001 },
    { ITEM, 0, 20000, 1000, 0, { 4, { 'A', 'B', 'C', 0 } } },
    { ITEM, 0, 20001, 1000, 0, { 12, 'D', 'E', 'F', 'G', 'H', 0 } },
    { ITEM, 0, 20002, 1001, 0, { 16, 'I', 'J', 'K', 'L', 0 } },
    { ITEM, 0, 20003, 1001, 0, { 10, 'M', 'N', 'O', 0 } },
  };

  TeamIndexData indexData[] = {
    { 1000, 0 }, { 1001, 2 },
  };

  size_t itemTeamOrderedIndex[] = {
    32, 72, 120,
  };

  ItemIdIndex itemIdIndex[] = {
    { 20000, 32 }, { 20001, 72 }, { 20002, 120 },
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

  cout << endl;

  auto itemOffset = binarySearch(itemIdIndex, sizeof(itemIdIndex) / sizeof(ItemIdIndex), 20001);

  if (itemOffset >= 0) {
    int8_t *ptr = reinterpret_cast<int8_t *>(&data) + itemOffset;

    cout << getInt32(ptr, 0) << "\t" << getInt32(ptr, 8) << endl;
  }

  itemOffset = binarySearch2(indexData, sizeof(indexData) / sizeof(TeamIndexData), 1001);

  cout << itemOffset << endl;
  
  ptr = reinterpret_cast<int8_t *>(&data) + itemTeamOrderedIndex[indexData[itemOffset].firstIndex];

  cout << getInt32(ptr, 0) << "\t" << getInt32(ptr, 8) << endl;

  ptr += sizeof(Item<0>) + (getInt32(ptr, 28) + 8 - 1) / 8 * 8;

  cout << getInt32(ptr, 0) << "\t" << getInt32(ptr, 8) << endl;

  return 0;
}

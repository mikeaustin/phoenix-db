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

void print(const Record& record) {
    size_t offset = 0;

    for (const Column *field = record.table->columns; field->type != Primitive::NVALID; ++field) {
        switch (field->type) {
            case Primitive::NVALID:
            case Primitive::TYPE32:
                break;
            case Primitive::UINT32:
                cout << format(field->name, getInt<uint32_t>(record.data, offset)) << endl;
                break;
            case Primitive::UINT64:
                cout << format(field->name, getInt<uint64_t>(record.data, offset)) << endl;
                break;
            case Primitive::STRING:
                auto string = getString(record.data, offset);
                cout << format(field->name, string.data) << endl;
                offset += (string.length + 8 - 1) / 8 * 8;
                break;
        }

        offset += Primitive::sizes[static_cast<size_t>(field->type)];
    }
}

size_t recordSize(const Record& record) {
    size_t fieldOffset = 0;

    for (const Column *field = record.table->columns; field->type != Primitive::NVALID; ++field) {
        switch (field->type) {
            case Primitive::NVALID:
            case Primitive::TYPE32:
            case Primitive::UINT32:
            case Primitive::UINT64:
                break;
            case Primitive::STRING:
                auto string = getString(record.data, fieldOffset);
                fieldOffset += (string.length + 8 - 1) / 8 * 8;
                break;
        }

        fieldOffset += Primitive::sizes[static_cast<size_t>(field->type)];
    }

    return fieldOffset;
}

void printRow(size_t offset, const Record& record) {
    size_t fieldOffset = 0;

    cout << std::left << std::setw(16) << offset;

    for (const Column *field = record.table->columns; field->type != Primitive::NVALID; ++field) {
        switch (field->type) {
            case Primitive::NVALID:
            case Primitive::TYPE32:
                break;
            case Primitive::UINT32:
                cout << std::left << std::setw(16) << getInt<uint32_t>(record.data, fieldOffset);
                break;
            case Primitive::UINT64:
                cout << std::left << std::setw(16) << getInt<uint64_t>(record.data, fieldOffset);
                break;
            case Primitive::STRING:
                auto string = getString(record.data, fieldOffset);
                cout << std::left << std::setw(16) << std::setw(0) << string.data << " (" << string.length << ")";
                fieldOffset += (string.length + 8 - 1) / 8 * 8;
                break;
        }

        fieldOffset += Primitive::sizes[static_cast<size_t>(field->type)];
    }

    cout << endl;
}

void dump(Table table) {
    cout << table.name << endl << endl;

    cout << std::left << std::setw(16) << "offset";

    int i = 0;
    for (const Column *column = table.columns; column->type != Primitive::NVALID; ++column) {
        cout << std::setw(16) << column->name;
        ++i;
    }

    cout << endl << repeat("===============", i + 1) << endl;

    auto first = table.data,
         last = table.data + table.size,
         record = first;

    while (record < last) {
        printRow(record - first, { &table, record });

        record += recordSize({ &table, record });
    }
}

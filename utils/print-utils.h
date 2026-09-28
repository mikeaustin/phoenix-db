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
    ostream << std::left;

    ostream << std::setw(16) << wrapper.a; if (!&wrapper.b) return ostream;
    ostream << std::setw(16) << wrapper.b; if (!&wrapper.c) return ostream;
    ostream << std::setw(16) << wrapper.c; if (!&wrapper.d) return ostream;
    ostream << std::setw(16) << wrapper.d; if (!&wrapper.e) return ostream;
    ostream << std::setw(16) << wrapper.e; if (!&wrapper.f) return ostream;
    ostream << std::setw(16) << wrapper.f; if (!&wrapper.g) return ostream;
    ostream << std::setw(16) << wrapper.g;

    return ostream;
}

template <typename T1, typename T2 = void *, typename T3 = void *, typename T4 = void *, typename T5 = void *, typename T6 = void *, typename T7 = void *>
FormatWrapper<T1, T2, T3, T4, T5, T6, T7> format(const T1& a, const T2& b = nullptr , const T3& c = nullptr, const T4& d = nullptr, const T5& e = nullptr, const T6& f = nullptr, const T7& g = nullptr) {
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

    for (auto field = record.table->columns.begin(); field != record.table->columns.end(); ++field) {
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

    for (auto field = record.table->columns.begin(); field != record.table->columns.end(); ++field) {
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

void printRow(const Record& record, const std::vector<Field>& fields = {}) {
    size_t fieldOffset = 0;

    cout << std::left << std::setw(16) << record.data - record.table->data;

    for (auto field = record.table->columns.begin(); field != record.table->columns.end(); ++field) {
        // if (std::find(fields.begin(), fields.end(), string(field->name)) != fields.end()) {
        //     continue;
        // }

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
                cout << string.data << " (" << string.length << ")";
                fieldOffset += (string.length + 8 - 1) / 8 * 8;
                break;
        }

        fieldOffset += Primitive::sizes[static_cast<size_t>(field->type)];
    }

    cout << endl;
}

void dump(const Table *table, const std::vector<Field>& fields = {}) {
    cout << table->name << endl << endl;

    if (fields.size() > 0) {
        for (auto field = fields.begin(); field != fields.end(); ++field) {
            cout << field->name << " ";
        }
        cout << endl << endl;
    }

    cout << std::left << std::setw(16) << "offset";

    for (auto column = table->columns.begin(); column != table->columns.end(); ++column) {
        cout << std::setw(16) << column->name;
    }

    cout << endl << repeat("===============", table->columns.size() + 1) << endl;

    auto first = table->data,
         last = table->data + table->size,
         record = first;

    while (record < last) {
        printRow({ table, record }, fields);

        record += recordSize({ table, record });
    }
}

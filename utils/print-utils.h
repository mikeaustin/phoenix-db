#ifndef PRINT_UTILS
#define PRINT_UTILS

template <typename... Args>
void format(const Args&... args) {
    ((std::cout << std::setw(16) << args), ...);

    std::cout << endl;
}

template <typename T>
void repeat(const T& arg, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        std::cout << std::left << std::setw(16) << arg;
    }

    std::cout << endl;
}

template <typename... Args>
void header(const Args&... args) {
    format(args...);

    repeat("===============", sizeof...(args));
}

size_t recordSize(const Record& record) {
    size_t fieldOffset = 0;

    for (auto field = record.table->columns.begin(); field != record.table->columns.end(); ++field) {
        switch (field->type) {
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

void printRecord(const Record& record, const std::vector<Field>& fields = {}) {
    size_t fieldOffset = 0;

    cout << std::left << std::setw(16) << record.data - record.table->data;

    for (auto field = record.table->columns.begin(); field != record.table->columns.end(); ++field) {
        auto it = std::find_if(fields.begin(), fields.end(), [field](const Field& _field) {
            return _field.name == field->name;
        });

        if (fields.size() == 0 || it != fields.end()) {
            switch (field->type) {
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
                    break;
            }
        }

        if (field->type == Primitive::STRING) {
            auto string = getString(record.data, fieldOffset);

            fieldOffset += (string.length + 8 - 1) / 8 * 8;
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

    cout << endl;

    repeat("===============", table->columns.size() + 1);

    auto first = table->data,
         last = table->data + table->size,
         row = first;

    while (row < last) {
        printRecord({ table, row }, fields);

        row += recordSize({ table, row });
    }
}

#endif

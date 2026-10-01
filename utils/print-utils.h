#ifndef PRINT_UTILS
#define PRINT_UTILS

#include <iostream>

using std::cout;

template <typename... Args>
void print(const Args&... args) {
    ((cout << std::setw(16) << args), ...);

    cout << endl;
}

template <typename T>
void repeat(const T& arg, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        cout << std::left << std::setw(16) << arg;
    }

    cout << endl;
}

void printHeader(const Table& table, const vector<Field>& fields = {}, const string& title = "") {
    if (title.size() > 0) {
        cout << title << endl << endl;
    }

    cout << std::left << std::setw(16) << "offset";

    for (auto column = table.columns.begin(); column != table.columns.end(); ++column) {
        auto it = find_if(fields.begin(), fields.end(), [column](const Field& _columns) {
            return _columns.name == column->name;
        });

        if (fields.size() == 0 || it != fields.end()) {
            cout << std::setw(16) << column->name;
        }
    }

    cout << endl;

    repeat("===============", fields.size() > 0 ? fields.size() + 1 : table.columns.size() + 1);
}

size_t recordSize(const Record& record) {
    size_t columnOffset = 0;

    for (auto column = record.table->columns.begin(); column != record.table->columns.end(); ++column) {
        switch (column->type) {
            case Primitive::TYPE32:
            case Primitive::UINT32:
            case Primitive::UINT64:
                break;
            case Primitive::STRING:
                auto string = getString(record.data, columnOffset);
                columnOffset += (string.length + 8 - 1) / 8 * 8;
                break;
        }

        columnOffset += Primitive::sizes[static_cast<size_t>(column->type)];
    }

    return columnOffset;
}

void printRecord(const Record& record, const vector<Field>& fields = {}) {
    size_t columnOffset = 0;

    cout << std::left << std::setw(16) << record.data - record.table->data;

    for (auto column = record.table->columns.begin(); column != record.table->columns.end(); ++column) {
        auto it = find_if(fields.begin(), fields.end(), [column](const Field& _column) {
            return _column.name == column->name;
        });

        if (fields.size() == 0 || it != fields.end()) {
            switch (column->type) {
                case Primitive::TYPE32:
                    break;
                case Primitive::UINT32:
                    cout << std::left << std::setw(16) << getInt<uint32_t>(record.data, columnOffset);
                    break;
                case Primitive::UINT64:
                    cout << std::left << std::setw(16) << getInt<uint64_t>(record.data, columnOffset);
                    break;
                case Primitive::STRING:
                    auto string = getString(record.data, columnOffset);
                    cout << string.data << " (" << string.length << ")";
                    break;
            }
        }

        if (column->type == Primitive::STRING) {
            auto string = getString(record.data, columnOffset);

            columnOffset += (string.length + 8 - 1) / 8 * 8;
        }

        columnOffset += Primitive::sizes[static_cast<size_t>(column->type)];
    }

    cout << endl;
}

void dumpTable(const Table *table, const vector<Field>& fields = {}, const string& title = "") {
    printHeader(*table, fields, title);

    auto first = table->data,
         last = table->data + table->size,
         row = first;

    while (row < last) {
        printRecord({ table, row }, fields);

        row += recordSize({ table, row });
    }
}

void dumpTableByNonUniqueIndex(const Table *itemsTable, NonUniqueIndex<uint64_t, 3, 4> itemTeamIdIndex, uint64_t index) {
    auto firstIndex = itemTeamIdIndex.indexes[index];
    auto lastIndex = sizeof(itemTeamIdIndex.offsets) / sizeof(size_t);

    for (size_t offsetIndex = firstIndex; offsetIndex < lastIndex; ++offsetIndex) {
        auto row = getRow(itemsTable->data, itemTeamIdIndex.offsets[offsetIndex]);

        if (offsetIndex >= itemTeamIdIndex.indexes[index + 1]) {
            break;
        }

        printRecord({ itemsTable, row });
    };
}

#endif

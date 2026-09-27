#include <iostream>

using std::string;

struct Keyword {
    enum Value : int8_t {
        SELECT = 'S',
    };

    Value value;
};

struct Identifier {
    string value;
};

struct Field {
    const string name;
};

struct Statement {
    Keyword keyword;
    Identifier table;
    std::vector<Field> fields;
};

void whitespace(string::iterator& input, string::iterator end) {
    while (input != end && std::isspace(*input)) {
        ++input;
    }
}

bool peek(const char value, string::iterator& input, string::iterator end) {
    if (input != end && *input == value) {
        return true;
    }

    return false;
}

bool expect(const char value, string::iterator& input, string::iterator end) {
    if (input != end && *input == value) {
        ++input;

        return true;
    }

    throw string("Expected ") + value;
}

bool match(const char value, string::iterator& input, string::iterator end) {
    if (input != end && *input == value) {
        ++input;

        return true;
    }

    return false;
}

std::optional<Keyword> keyword(string::iterator& input, string::iterator end) {
    string keyword(input, input + 6);

    if (keyword == "select") {
        input += 6;

        return Keyword { Keyword::SELECT };
    }

    return std::nullopt;
}

std::optional<Identifier> identifier(string::iterator& input, string::iterator end) {
    string chars = ", ";

    auto space_it = std::find_first_of(input, end, chars.begin(), chars.end());

    string identifier(input, space_it);

    input += std::distance(input, space_it);

    return Identifier { identifier };
}

std::optional<std::vector<Field>> fields(string::iterator& input, string::iterator end) {
    if (!peek('{', input, end)) {
        return std::nullopt;
    }

    std::vector<Field> fields;

    expect('{', input, end);

    while (true) {
        whitespace(input, end);

        auto _identifier = identifier(input, end);

        if (_identifier) {
            fields.push_back(Field { _identifier->value });
        }

        whitespace(input, end);

        if (!match(',', input, end)) {
            break;
        }
    }
    whitespace(input, end);
    expect('}', input, end);

    return fields;
}

Statement expression(string::iterator& input, string::iterator end) {
    auto _keyword = keyword(input, end);
    whitespace(input, end);
    auto _table = identifier(input, end);
    whitespace(input, end);
    auto _fields = fields(input, end);

    if (_table) {
        return Statement { *_keyword, *_table };
    }

    return Statement { };
}

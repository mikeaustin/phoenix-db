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

struct Statement {
    Keyword keyword;
    Identifier table;
};

void whitespace(string::iterator& input, string::iterator end) {
    while (input != end && std::isspace(*input)) {
        ++input;
    }
}

void expect(const char value, string::iterator& input, string::iterator end) {
    if (input != end && *input == value) {
        throw "nope";
    }
}

bool match(const char value, string::iterator& input, string::iterator end) {
    if (input != end && *input == value) {
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
    auto space_it = std::find(input, end, ' ');

    string identifier(input, space_it);

    input += std::distance(input, space_it);

    return Identifier { identifier };
}

std::optional<Identifier> fields(string::iterator& input, string::iterator end) {
    expect('{', input, end);
    expect('}', input, end);

    return std::nullopt;
}

Statement expression(string::iterator& input, string::iterator end) {
    auto _keyword = keyword(input, end);
    whitespace(input, end);
    auto _table = identifier(input, end);

    if (_table) {
        return Statement { *_keyword, *_table };
    }

    return Statement { };
}

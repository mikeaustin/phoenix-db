#ifndef QUERY_UTILS
#define QUERY_UTILS

#include <iostream>

using std::string;

struct Value {

};

struct KopiString {
    string value;
};

struct ASTNode {
    enum Type {
        Identifier = 1,
    };

    // virtual Value evaluate() = 0;
    int type;
};

struct Keyword {
    enum Value : int8_t {
        SELECT = 'S',
    };

    Value value;
};

struct Identifier : public ASTNode {
    Identifier(string value) : ASTNode(ASTNode::Identifier) {
        this->value = value;
    }

    Value evaluate() {
        return Value { };
    }

    string value;
};

struct Field {
    const string name;
};

struct Statement {
    Keyword keyword;
    Identifier table;
    const std::vector<Field> fields;
};

void whitespace(string::iterator& input, string::iterator end) {
    while (input != end && std::isspace(*input)) {
        ++input;
    }
}

bool peek(const char value, string::iterator& input, string::iterator end) {
    whitespace(input, end);
    
    if (input != end && *input == value) {
        return true;
    }

    return false;
}

bool expect(const char value, string::iterator& input, string::iterator end) {
    whitespace(input, end);

    if (input != end && *input == value) {
        ++input;

        return true;
    }

    throw string("Expected ") + value;
}

bool match(const char value, string::iterator& input, string::iterator end) {
    whitespace(input, end);

    if (input != end && *input == value) {
        ++input;

        return true;
    }

    return false;
}

std::optional<Keyword> keyword(string::iterator& input, string::iterator end) {
    whitespace(input, end);

    string keyword(input, input + 6);

    if (keyword == "select") {
        input += 6;

        return Keyword { Keyword::SELECT };
    }

    return std::nullopt;
}

std::optional<Identifier> identifier(string::iterator& input, string::iterator end) {
    string identifier = "";

    identifier.reserve(20);

    whitespace(input, end);

    while (input != end && std::isalpha(*input)) {
        identifier += *input++;
    }

    if (identifier.size() > 0) {
        return Identifier { identifier };
    }

    return std::nullopt;
}

std::optional<std::vector<Field>> fields(string::iterator& input, string::iterator end) {
    if (!peek('{', input, end)) {
        return std::nullopt;
    }

    std::vector<Field> fields;

    expect('{', input, end);

    while (true) {
        auto _identifier = identifier(input, end);

        if (_identifier) {
            fields.push_back(Field { _identifier->value });
        }

        if (!match(',', input, end)) {
            break;
        }
    }

    expect('}', input, end);

    return fields;
}

Statement expression(string::iterator& input, string::iterator end) {
    auto _keyword = keyword(input, end);
    auto _table = identifier(input, end);
    auto _fields = fields(input, end);

    if (_table) {
        return Statement { *_keyword, *_table, _fields ? *_fields : *new std::vector<Field>() };
    }

    throw "Error";
}

void evaluate(const ASTNode& node) {
    switch (node.type) {
        case ASTNode::Identifier:
            const Identifier& _node = static_cast<const Identifier&>(node);

            cout << _node.value << endl;
        break;
    }
}

#endif

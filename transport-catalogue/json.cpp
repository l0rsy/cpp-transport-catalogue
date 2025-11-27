#include "json.h"

#include <cmath>
#include <iomanip>
#include <sstream>

using namespace std;

namespace json {

// Конструкторы Node
Node::Node() : value_(nullptr) {}
Node::Node(nullptr_t) : value_(nullptr) {}
Node::Node(Array array) : value_(move(array)) {}
Node::Node(Dict map) : value_(move(map)) {}
Node::Node(int value) : value_(value) {}
Node::Node(double value) : value_(value) {}
Node::Node(bool value) : value_(value) {}
Node::Node(string value) : value_(move(value)) {}
Node::Node(const char* value) : value_(string(value)) {}

// Методы проверки типов
bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(value_);
}

bool Node::IsInt() const {
    return holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}

bool Node::IsPureDouble() const {
    return holds_alternative<double>(value_);
}

bool Node::IsBool() const {
    return holds_alternative<bool>(value_);
}

bool Node::IsString() const {
    return holds_alternative<string>(value_);
}

bool Node::IsArray() const {
    return holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return holds_alternative<Dict>(value_);
}

// Методы получения значений
int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("Not an int");
    }
    return get<int>(value_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("Not a bool");
    }
    return get<bool>(value_);
}

double Node::AsDouble() const {
    if (IsInt()) {
        return static_cast<double>(get<int>(value_));
    }
    if (IsPureDouble()) {
        return get<double>(value_);
    }
    throw logic_error("Not a double");
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw logic_error("Not a string");
    }
    return get<string>(value_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("Not an array");
    }
    return get<Array>(value_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw logic_error("Not a map");
    }
    return get<Dict>(value_);
}

const Node::Value& Node::GetValue() const {
    return value_;
}

// Операторы сравнения
bool Node::operator==(const Node& other) const {
    return value_ == other.value_;
}

bool Node::operator!=(const Node& other) const {
    return !(*this == other);
}

// Document implementation
Document::Document(Node root) : root_(move(root)) {}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}

bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

namespace {

Node LoadNode(istream& input);

void SkipWhitespace(istream& input) {
    while (isspace(input.peek())) {
        input.get();
    }
}

string ParseEscapeSequences(istream& input) {
    char c = input.get();
    switch (c) {
        case 'n': return "\n";
        case 'r': return "\r";
        case 't': return "\t";
        case '"': return "\"";
        case '\\': return "\\";
        default:
            throw ParsingError("Invalid escape sequence: \\"s + c);
    }
}

Node LoadString(istream& input) {
    string result;
    
    while (true) {
        char c = input.get();
        if (!input) {
            throw ParsingError("String parsing error");
        }
        
        if (c == '"') {
            break;
        } else if (c == '\\') {
            result += ParseEscapeSequences(input);
        } else {
            result += c;
        }
    }
    
    return Node(move(result));
}

Node LoadNumber(istream& input) {
    string parsed_num;
    
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream");
        }
    };
    
    if (input.peek() == '-') {
        read_char();
    }
    
    if (input.peek() == '0') {
        read_char();
    } else if (isdigit(input.peek())) {
        while (isdigit(input.peek())) {
            read_char();
        }
    } else {
        throw ParsingError("Invalid number");
    }
    
    bool is_double = false;
    if (input.peek() == '.') {
        read_char();
        is_double = true;
        
        if (!isdigit(input.peek())) {
            throw ParsingError("Invalid number");
        }
        
        while (isdigit(input.peek())) {
            read_char();
        }
    }
    
    if (input.peek() == 'e' || input.peek() == 'E') {
        read_char();
        is_double = true;
        
        if (input.peek() == '+' || input.peek() == '-') {
            read_char();
        }
        
        if (!isdigit(input.peek())) {
            throw ParsingError("Invalid number");
        }
        
        while (isdigit(input.peek())) {
            read_char();
        }
    }
    
    try {
        if (is_double) {
            return Node(stod(parsed_num));
        } else {
            return Node(stoi(parsed_num));
        }
    } catch (...) {
        throw ParsingError("Failed to convert to number: " + parsed_num);
    }
}

Node LoadArray(istream& input) {
    Array result;
    
    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    
    if (!input) {
        throw ParsingError("Array parsing error");
    }
    
    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;
    
    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        
        if (c != '"') {
            throw ParsingError("Expected '\"' in dict key");
        }
        
        string key = LoadString(input).AsString();
        
        input >> c;
        if (c != ':') {
            throw ParsingError("Expected ':' after dict key");
        }
        
        result.insert({move(key), LoadNode(input)});
    }
    
    if (!input) {
        throw ParsingError("Dict parsing error");
    }
    
    return Node(move(result));
}

Node LoadNull(istream& input) {
    string s;
    
    while (isalpha(input.peek())) {
        s += static_cast<char>(input.get());
    }
    if (s != "null") {
        throw ParsingError("Invalid null value: " + s);
    }
    
    return Node(nullptr);
}

Node LoadBool(istream& input) {
    string s;

    while (isalpha(input.peek())) {
        s += static_cast<char>(input.get());
    }
    if (s == "true") {
        return Node(true);
    } else if (s == "false") {
        return Node(false);
    } else {
        throw ParsingError("Invalid bool value: " + s);
    }
}

Node LoadNode(istream& input) {
    SkipWhitespace(input);
    char c = input.peek();
    
    if (c == '[') {
        input.get();
        return LoadArray(input);
    } else if (c == '{') {
        input.get();
        return LoadDict(input);
    } else if (c == '"') {
        input.get();
        return LoadString(input);
    } else if (c == 'n') {
        return LoadNull(input);
    } else if (c == 't' || c == 'f') {
        return LoadBool(input);
    } else if (c == '-' || isdigit(c)) {
        return LoadNumber(input);
    } else {
        throw ParsingError("Unexpected character: "s + c);
    }
}

}  // namespace

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

struct PrintContext {
    ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }
};

void PrintValue(nullptr_t, const PrintContext& ctx) {
    ctx.out << "null";
}

void PrintValue(bool value, const PrintContext& ctx) {
    ctx.out << (value ? "true" : "false");
}

void PrintValue(int value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue(double value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue(const string& value, const PrintContext& ctx) {
    ctx.out << '"';
    for (char c : value) {
        switch (c) {
            case '\n': ctx.out << "\\n"; break;
            case '\r': ctx.out << "\\r"; break;
            case '\t': ctx.out << "\\t"; break;
            case '"': ctx.out << "\\\""; break;
            case '\\': ctx.out << "\\\\"; break;
            default: ctx.out << c;
        }
    }
    ctx.out << '"';
}

void PrintValue(const Array& array, const PrintContext& ctx) {
    ctx.out << "[\n";
    bool first = true;
    auto inner_ctx = ctx.Indented();
    
    for (const auto& item : array) {
        if (!first) {
            ctx.out << ",\n";
        }
        first = false;
        
        inner_ctx.PrintIndent();
        visit([&inner_ctx](const auto& value) { PrintValue(value, inner_ctx); }, 
              item.GetValue());
    }
    
    if (!array.empty()) {
        ctx.out << "\n";
        ctx.PrintIndent();
    }
    ctx.out << "]";
}

void PrintValue(const Dict& dict, const PrintContext& ctx) {
    ctx.out << "{\n";
    bool first = true;
    auto inner_ctx = ctx.Indented();
    
    for (const auto& [key, value] : dict) {
        if (!first) {
            ctx.out << ",\n";
        }
        first = false;
        
        inner_ctx.PrintIndent();
        PrintValue(key, inner_ctx);
        ctx.out << ": ";
        visit([&inner_ctx](const auto& val) { PrintValue(val, inner_ctx); }, 
              value.GetValue());
    }
    
    if (!dict.empty()) {
        ctx.out << "\n";
        ctx.PrintIndent();
    }
    ctx.out << "}";
}

void Print(const Document& doc, ostream& output) {
    PrintContext ctx{output};
    visit([&ctx](const auto& value) { PrintValue(value, ctx); }, 
          doc.GetRoot().GetValue());
}

}  // namespace json
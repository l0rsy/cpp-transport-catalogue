#pragma once

#include "json.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace json {

class Builder {
public:
    Builder() = default;

    // Контекстные классы
    class BaseContext;
    class DictItemContext;
    class DictKeyContext;
    class ArrayItemContext;

    // Методы Builder
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& Value(Node value);
    Builder& EndDict();
    Builder& EndArray();
    DictKeyContext Key(std::string key);

    // Шаблонные методы Value для разных типов
    template<typename T>
    Builder& Value(T&& value) {
        return Value(Node(std::forward<T>(value)));
    }

    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::vector<std::string> keys_;
    bool expecting_value_ = false;
    
    void AddValue(Node value);
    Node* GetCurrentNode();
    bool IsDictContext() const;
    bool IsArrayContext() const;
    bool IsComplete() const;
};

// Базовый класс для всех контекстов (только объявление, определение после)
class Builder::BaseContext {
protected:
    BaseContext(Builder& builder);
    
public:
    // Только объявления
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    DictKeyContext Key(std::string key);
    
    DictItemContext Value(Node value);
    
    DictItemContext Value(int value);
    DictItemContext Value(double value);
    DictItemContext Value(bool value);
    DictItemContext Value(const char* value);
    DictItemContext Value(std::string value);
    DictItemContext Value(std::nullptr_t);
    DictItemContext Value(Array value);
    DictItemContext Value(Dict value);

protected:
    Builder& builder_;
};

// Контекстные классы (определения inline)
class Builder::DictItemContext : public BaseContext {
public:
    DictItemContext(Builder& builder);
    
    // В словаре можно Key или EndDict
    DictKeyContext Key(std::string key);
    Builder& EndDict();
    
    // Остальные методы запрещаем
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
};

class Builder::DictKeyContext : public BaseContext {
public:
    DictKeyContext(Builder& builder);
    
    // После Key можно Value, StartDict или StartArray
    DictItemContext Value(Node value);
    DictItemContext Value(int value);
    DictItemContext Value(double value);
    DictItemContext Value(bool value);
    DictItemContext Value(const char* value);
    DictItemContext Value(std::string value);
    DictItemContext Value(std::nullptr_t);
    DictItemContext Value(Array value);
    DictItemContext Value(Dict value);
    
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    
    // Запрещаем методы, которые не должны быть доступны
    DictKeyContext Key(std::string) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};

class Builder::ArrayItemContext : public BaseContext {
public:
    ArrayItemContext(Builder& builder);
    
    // В массиве можно Value, StartDict, StartArray или EndArray
    ArrayItemContext Value(Node value);
    ArrayItemContext Value(int value);
    ArrayItemContext Value(double value);
    ArrayItemContext Value(bool value);
    ArrayItemContext Value(const char* value);
    ArrayItemContext Value(std::string value);
    ArrayItemContext Value(std::nullptr_t);
    ArrayItemContext Value(Array value);
    ArrayItemContext Value(Dict value);
    
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    
    // Запрещаем методы, которые не должны быть доступны
    DictKeyContext Key(std::string) = delete;
    Builder& EndDict() = delete;
};

// Определения методов BaseContext
inline Builder::BaseContext::BaseContext(Builder& builder) : builder_(builder) {}

inline Builder::DictItemContext Builder::BaseContext::StartDict() {
    return builder_.StartDict();
}

inline Builder::ArrayItemContext Builder::BaseContext::StartArray() {
    return builder_.StartArray();
}

inline Builder& Builder::BaseContext::EndDict() {
    return builder_.EndDict();
}

inline Builder& Builder::BaseContext::EndArray() {
    return builder_.EndArray();
}

inline Builder::DictKeyContext Builder::BaseContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

inline Builder::DictItemContext Builder::BaseContext::Value(Node value) {
    builder_.Value(std::move(value));
    return DictItemContext(builder_);
}

inline Builder::DictItemContext Builder::BaseContext::Value(int value) {
    return Value(Node(value));
}

inline Builder::DictItemContext Builder::BaseContext::Value(double value) {
    return Value(Node(value));
}

inline Builder::DictItemContext Builder::BaseContext::Value(bool value) {
    return Value(Node(value));
}

inline Builder::DictItemContext Builder::BaseContext::Value(const char* value) {
    return Value(Node(std::string(value)));
}

inline Builder::DictItemContext Builder::BaseContext::Value(std::string value) {
    return Value(Node(std::move(value)));
}

inline Builder::DictItemContext Builder::BaseContext::Value(std::nullptr_t) {
    return Value(Node(nullptr));
}

inline Builder::DictItemContext Builder::BaseContext::Value(Array value) {
    return Value(Node(std::move(value)));
}

inline Builder::DictItemContext Builder::BaseContext::Value(Dict value) {
    return Value(Node(std::move(value)));
}

// Определения методов контекстных классов
inline Builder::DictItemContext::DictItemContext(Builder& builder) : BaseContext(builder) {}

inline Builder::DictKeyContext Builder::DictItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

inline Builder& Builder::DictItemContext::EndDict() {
    return builder_.EndDict();
}

inline Builder::DictKeyContext::DictKeyContext(Builder& builder) : BaseContext(builder) {}

inline Builder::DictItemContext Builder::DictKeyContext::Value(Node value) {
    builder_.Value(std::move(value));
    return DictItemContext(builder_);
}

inline Builder::DictItemContext Builder::DictKeyContext::Value(int value) {
    return Value(Node(value));
}

inline Builder::DictItemContext Builder::DictKeyContext::Value(double value) {
    return Value(Node(value));
}

inline Builder::DictItemContext Builder::DictKeyContext::Value(bool value) {
    return Value(Node(value));
}

inline Builder::DictItemContext Builder::DictKeyContext::Value(const char* value) {
    return Value(Node(std::string(value)));
}

inline Builder::DictItemContext Builder::DictKeyContext::Value(std::string value) {
    return Value(Node(std::move(value)));
}

inline Builder::DictItemContext Builder::DictKeyContext::Value(std::nullptr_t) {
    return Value(Node(nullptr));
}

inline Builder::DictItemContext Builder::DictKeyContext::Value(Array value) {
    return Value(Node(std::move(value)));
}

inline Builder::DictItemContext Builder::DictKeyContext::Value(Dict value) {
    return Value(Node(std::move(value)));
}

inline Builder::DictItemContext Builder::DictKeyContext::StartDict() {
    return builder_.StartDict();
}

inline Builder::ArrayItemContext Builder::DictKeyContext::StartArray() {
    return builder_.StartArray();
}

inline Builder::ArrayItemContext::ArrayItemContext(Builder& builder) : BaseContext(builder) {}

inline Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node value) {
    builder_.Value(std::move(value));
    return ArrayItemContext(builder_);
}

inline Builder::ArrayItemContext Builder::ArrayItemContext::Value(int value) {
    return Value(Node(value));
}

inline Builder::ArrayItemContext Builder::ArrayItemContext::Value(double value) {
    return Value(Node(value));
}

inline Builder::ArrayItemContext Builder::ArrayItemContext::Value(bool value) {
    return Value(Node(value));
}

inline Builder::ArrayItemContext Builder::ArrayItemContext::Value(const char* value) {
    return Value(Node(std::string(value)));
}

inline Builder::ArrayItemContext Builder::ArrayItemContext::Value(std::string value) {
    return Value(Node(std::move(value)));
}

inline Builder::ArrayItemContext Builder::ArrayItemContext::Value(std::nullptr_t) {
    return Value(Node(nullptr));
}

inline Builder::ArrayItemContext Builder::ArrayItemContext::Value(Array value) {
    return Value(Node(std::move(value)));
}

inline Builder::ArrayItemContext Builder::ArrayItemContext::Value(Dict value) {
    return Value(Node(std::move(value)));
}

inline Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

inline Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

inline Builder& Builder::ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

} // namespace json
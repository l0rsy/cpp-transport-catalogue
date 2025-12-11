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
    class DictItemContext;
    class DictKeyContext;
    class ArrayItemContext;

    // Методы, доступные из корневого контекста
    DictItemContext StartDict();
    ArrayItemContext StartArray();

    Builder& Value(Node value);
    Builder& EndDict();
    Builder& EndArray();

    DictKeyContext Key(std::string key);

    Builder& Value(int value) {
        return Value(Node(value));
    }
    
    Builder& Value(double value) {
        return Value(Node(value));
    }
    
    Builder& Value(bool value) {
        return Value(Node(value));
    }
    
    Builder& Value(const char* value) {
        return Value(Node(std::string(value)));
    }
    
    Builder& Value(std::string value) {
        return Value(Node(std::move(value)));
    }
    
    Builder& Value(std::nullptr_t) {
        return Value(Node(nullptr));
    }

    Builder& Value(Array value) {
        return Value(Node(std::move(value)));
    }

    Builder& Value(Dict value) {
        return Value(Node(std::move(value)));
    }
    
    Node Build();
    
private:
    // Приватные данные для управления состоянием
    Node root_;
    std::vector<Node*> nodes_stack_;  // Стек указателей на узлы
    std::vector<std::string> keys_;   // Стек ключей для словарей
    bool expecting_value_ = false;    // Ожидаем ли значение после ключа
    
    void AddValue(Node value);
    
    Node* GetCurrentNode();
    bool IsDictContext() const;
    bool IsArrayContext() const;
    bool IsComplete() const;
};

// Базовый класс контекста
class Builder::DictItemContext {
public:
    DictItemContext(Builder& builder) : builder_(builder) {}
    
    // В словаре можно только Key или EndDict
    DictKeyContext Key(std::string key);
    Builder& EndDict();
    
private:
    Builder& builder_;
};

// Контекст после Key в словаре
class Builder::DictKeyContext {
public:
    DictKeyContext(Builder& builder) : builder_(builder) {}
    
    // После Key можно Value, StartDict или StartArray
    DictItemContext Value(Node value);

    DictItemContext Value(int value) {
        return Value(Node(value));
    }
    
    DictItemContext Value(double value) {
        return Value(Node(value));
    }
    
    DictItemContext Value(bool value) {
        return Value(Node(value));
    }
    
    DictItemContext Value(std::string value) {
        return Value(Node(std::move(value)));
    }
    
    DictItemContext Value(const char* value) {
        return Value(Node(std::string(value)));
    }
    
    DictItemContext Value(std::nullptr_t) {
        return Value(Node(nullptr));
    }
    
    DictItemContext Value(Array value) {
        return Value(Node(std::move(value)));
    }
    
    DictItemContext Value(Dict value) {
        return Value(Node(std::move(value)));
    }
    
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    
private:
    Builder& builder_;
};

// Контекст для массива (после StartArray)
class Builder::ArrayItemContext {
public:
    ArrayItemContext(Builder& builder) : builder_(builder) {}
    
    // В массиве можно Value, StartDict, StartArray или EndArray
    ArrayItemContext Value(Node value);

    ArrayItemContext Value(int value) {
        return Value(Node(value));
    }
    
    ArrayItemContext Value(double value) {
        return Value(Node(value));
    }
    
    ArrayItemContext Value(bool value) {
        return Value(Node(value));
    }
    
    ArrayItemContext Value(std::string value) {
        return Value(Node(std::move(value)));
    }
    
    ArrayItemContext Value(const char* value) {
        return Value(Node(std::string(value)));
    }
    
    ArrayItemContext Value(std::nullptr_t) {
        return Value(Node(nullptr));
    }
    
    ArrayItemContext Value(Array value) {
        return Value(Node(std::move(value)));
    }
    
    ArrayItemContext Value(Dict value) {
        return Value(Node(std::move(value)));
    }
    
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    
private:
    Builder& builder_;
};

} // namespace json
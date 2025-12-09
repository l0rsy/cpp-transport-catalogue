#pragma once

#include "json.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace json {

// Вспомогательные классы контекстов
class BaseContext;
class DictItemContext;
class DictKeyContext;
class ArrayItemContext;

class Builder {
public:
    Builder() = default;
    
    // Методы, доступные из корневого контекста
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& Value(Node value);
    
    // Перегрузки Value для простых типов
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
    
    // Методы, доступные только из определенных контекстов
    DictKeyContext Key(std::string key);
    Builder& EndDict();
    Builder& EndArray();
    
    Node Build();
    
private:
    friend class BaseContext;
    friend class DictItemContext;
    friend class DictKeyContext;
    friend class ArrayItemContext;
    
    enum class State {
        EMPTY,
        DICT_KEY,
        DICT_VALUE,
        ARRAY,
        COMPLETE
    };
    
    struct Context {
        Node* node;
        State state;
    };
    
    Node root_;
    std::vector<Context> stack_;
    std::string last_key_;
    
    Node* GetCurrentNode();
    State GetCurrentState() const;
    void CheckNotComplete() const;
    void CheckComplete() const;
    
    // Вспомогательные методы для добавления значений в разных контекстах
    DictItemContext ValueInDict(Node value);
    ArrayItemContext ValueInArray(Node value);
};

// Базовый класс контекста
class BaseContext {
public:
    BaseContext(Builder& builder) : builder_(builder) {}
    
protected:
    Builder& builder_;
    
    // Запрещаем вызовы методов в неправильных контекстах
    Builder& StartDict() = delete;
    Builder& StartArray() = delete;
    Builder& Value(Node) = delete;
    Builder& Key(std::string) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};

// Контекст для словаря (после StartDict)
class DictItemContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    
    // В словаре можно только Key или EndDict
    DictKeyContext Key(std::string key);
    Builder& EndDict();
    
    // Запрещаем остальные методы
    Builder& StartDict() = delete;
    Builder& StartArray() = delete;
    Builder& Value(Node) = delete;
    Builder& EndArray() = delete;
};

// Контекст после Key в словаре
class DictKeyContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    
    // После Key можно Value, StartDict или StartArray
    DictItemContext Value(Node value);
    
    // Перегрузки для поддержки Array и Dict
    DictItemContext Value(Array value) {
        return Value(Node(std::move(value)));
    }
    
    DictItemContext Value(Dict value) {
        return Value(Node(std::move(value)));
    }
    
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
    
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    
    // Запрещаем остальные методы
    Builder& Key(std::string) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};

// Контекст для массива (после StartArray)
class ArrayItemContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    
    // В массиве можно Value, StartDict, StartArray или EndArray
    ArrayItemContext Value(Node value);
    
    // Перегрузки для поддержки Array и Dict
    ArrayItemContext Value(Array value) {
        return Value(Node(std::move(value)));
    }
    
    ArrayItemContext Value(Dict value) {
        return Value(Node(std::move(value)));
    }
    
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
    
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    
    // Запрещаем остальные методы
    Builder& Key(std::string) = delete;
    Builder& EndDict() = delete;
};

} // namespace json
#include "json_builder.h"
#include <sstream>

namespace json {

// Реализация методов Builder

DictItemContext Builder::StartDict() {
    CheckNotComplete();
    
    Node dict_node{Dict{}};
    
    if (stack_.empty()) {
        root_ = std::move(dict_node);
        stack_.push_back({&root_, State::DICT_KEY});
    } else {
        auto current_state = GetCurrentState();
        if (current_state == State::DICT_VALUE) {
            auto* current_node = GetCurrentNode();
            if (!current_node->IsMap()) {
                throw std::logic_error("Current node is not a dict");
            }
            
            auto& dict = const_cast<Dict&>(current_node->AsMap());
            auto it = dict.emplace(std::move(last_key_), std::move(dict_node)).first;
            last_key_.clear();
            stack_.back().state = State::DICT_KEY;
            stack_.push_back({&it->second, State::DICT_KEY});
            
        } else if (current_state == State::ARRAY) {
            auto* current_node = GetCurrentNode();
            if (!current_node->IsArray()) {
                throw std::logic_error("Current node is not an array");
            }
            
            auto& array = const_cast<Array&>(current_node->AsArray());
            array.emplace_back(std::move(dict_node));
            stack_.push_back({&array.back(), State::DICT_KEY});
            
        } else {
            throw std::logic_error("StartDict can only be called in dict after Key or in array");
        }
    }
    
    return DictItemContext(*this);
}

ArrayItemContext Builder::StartArray() {
    CheckNotComplete();
    
    Node array_node{Array{}};
    
    if (stack_.empty()) {
        root_ = std::move(array_node);
        stack_.push_back({&root_, State::ARRAY});
    } else {
        auto current_state = GetCurrentState();
        if (current_state == State::DICT_VALUE) {
            auto* current_node = GetCurrentNode();
            if (!current_node->IsMap()) {
                throw std::logic_error("Current node is not a dict");
            }
            
            auto& dict = const_cast<Dict&>(current_node->AsMap());
            auto it = dict.emplace(std::move(last_key_), std::move(array_node)).first;
            last_key_.clear();
            stack_.back().state = State::DICT_KEY;
            stack_.push_back({&it->second, State::ARRAY});
            
        } else if (current_state == State::ARRAY) {
            auto* current_node = GetCurrentNode();
            if (!current_node->IsArray()) {
                throw std::logic_error("Current node is not an array");
            }
            
            auto& array = const_cast<Array&>(current_node->AsArray());
            array.emplace_back(std::move(array_node));
            stack_.push_back({&array.back(), State::ARRAY});
            
        } else {
            throw std::logic_error("StartArray can only be called in dict after Key or in array");
        }
    }
    
    return ArrayItemContext(*this);
}

Builder& Builder::Value(Node value) {
    CheckNotComplete();
    
    if (stack_.empty()) {
        // Если стек пуст, это корневое значение
        root_ = std::move(value);
        stack_.push_back({&root_, State::COMPLETE});
        return *this;
    }
    
    auto current_state = GetCurrentState();
    if (current_state == State::DICT_VALUE) {
        // Добавляем значение в словарь
        if (last_key_.empty()) {
            throw std::logic_error("Key not set before value in dict");
        }
        
        auto* current_node = GetCurrentNode();
        if (!current_node->IsMap()) {
            throw std::logic_error("Current node is not a dict");
        }
        
        auto& dict = const_cast<Dict&>(current_node->AsMap());
        dict.emplace(std::move(last_key_), std::move(value));
        last_key_.clear();
        stack_.back().state = State::DICT_KEY;
        return *this;
        
    } else if (current_state == State::ARRAY) {
        // Добавляем значение в массив
        auto* current_node = GetCurrentNode();
        if (!current_node->IsArray()) {
            throw std::logic_error("Current node is not an array");
        }
        
        auto& array = const_cast<Array&>(current_node->AsArray());
        array.emplace_back(std::move(value));
        return *this;
    }
    
    throw std::logic_error("Value can only be called in dict after Key or in array");
}

DictKeyContext Builder::Key(std::string key) {
    CheckNotComplete();
    
    if (GetCurrentState() != State::DICT_KEY) {
        throw std::logic_error("Key can only be called inside a dict");
    }
    
    last_key_ = std::move(key);
    stack_.back().state = State::DICT_VALUE;
    
    return DictKeyContext(*this);
}

// json_builder.cpp - исправленный EndDict()
Builder& Builder::EndDict() {
    CheckNotComplete();
    
    if (GetCurrentState() != State::DICT_KEY) {
        // Допустим также DICT_VALUE? Нет, потому что тогда есть незавершенный ключ
        throw std::logic_error("EndDict can only be called inside a dict");
    }
    
    // Проверка незавершенного ключа
    if (GetCurrentState() == State::DICT_VALUE && !last_key_.empty()) {
        throw std::logic_error("Dict key without value");
    }
    
    stack_.pop_back();
    if (stack_.empty()) {
        // Если стек пуст, значит мы закончили корневой словарь
        stack_.push_back({&root_, State::COMPLETE});
    } else {
        // Обновляем состояние родительского элемента
        auto current_state = GetCurrentState();
        if (current_state == State::DICT_VALUE) {
            // Мы только что завершили значение для ключа в родительском словаре
            last_key_.clear();
            stack_.back().state = State::DICT_KEY;
        }
        // Для массива состояние остается ARRAY
    }
    
    return *this;
}
Builder& Builder::EndArray() {
    CheckNotComplete();
    
    if (GetCurrentState() != State::ARRAY) {
        throw std::logic_error("EndArray can only be called inside an array");
    }
    
    stack_.pop_back();
    if (stack_.empty()) {
        stack_.push_back({&root_, State::COMPLETE});
    }
    
    return *this;
}

Node Builder::Build() {
    CheckComplete();
    
    if (stack_.empty() || GetCurrentState() != State::COMPLETE) {
        throw std::logic_error("Attempt to build incomplete JSON");
    }
    
    return std::move(root_);
}

Node* Builder::GetCurrentNode() {
    if (stack_.empty()) {
        return &root_;
    }
    return stack_.back().node;
}

Builder::State Builder::GetCurrentState() const {
    if (stack_.empty()) {
        return State::EMPTY;
    }
    return stack_.back().state;
}

void Builder::CheckNotComplete() const {
    if (!stack_.empty() && stack_.back().state == State::COMPLETE) {
        throw std::logic_error("Builder is already complete");
    }
}

void Builder::CheckComplete() const {
    if (stack_.empty() || stack_.back().state != State::COMPLETE) {
        throw std::logic_error("JSON is not complete");
    }
}

// Вспомогательный метод для добавления значения в словаре
DictItemContext Builder::ValueInDict(Node value) {
    CheckNotComplete();
    
    if (GetCurrentState() != State::DICT_VALUE) {
        throw std::logic_error("Value can only be called after Key in dict");
    }
    
    auto* current_node = GetCurrentNode();
    if (!current_node->IsMap()) {
        throw std::logic_error("Current node is not a dict");
    }
    
    auto& dict = const_cast<Dict&>(current_node->AsMap());
    dict.emplace(std::move(last_key_), std::move(value));
    last_key_.clear();
    stack_.back().state = State::DICT_KEY;
    
    return DictItemContext(*this);
}

// Вспомогательный метод для добавления значения в массиве
ArrayItemContext Builder::ValueInArray(Node value) {
    CheckNotComplete();
    
    if (GetCurrentState() != State::ARRAY) {
        throw std::logic_error("Value can only be called in array");
    }
    
    auto* current_node = GetCurrentNode();
    if (!current_node->IsArray()) {
        throw std::logic_error("Current node is not an array");
    }
    
    auto& array = const_cast<Array&>(current_node->AsArray());
    array.emplace_back(std::move(value));
    
    return ArrayItemContext(*this);
}

// Реализация методов контекстов

DictKeyContext DictItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}

DictItemContext DictKeyContext::Value(Node value) {
    return builder_.ValueInDict(std::move(value));
}

DictItemContext DictKeyContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext DictKeyContext::StartArray() {
    return builder_.StartArray();
}

ArrayItemContext ArrayItemContext::Value(Node value) {
    return builder_.ValueInArray(std::move(value));
}

DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

} // namespace json
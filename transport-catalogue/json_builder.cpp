#include "json_builder.h"
#include <sstream>

namespace json {

Node* Builder::GetCurrentNode() {
    if (nodes_stack_.empty()) {
        return &root_;
    }
    return nodes_stack_.back();
}

bool Builder::IsDictContext() const {
    if (nodes_stack_.empty()) {
        // Корневой контекст - не словарь
        return false;
    }
    return nodes_stack_.back()->IsMap();
}

bool Builder::IsArrayContext() const {
    if (nodes_stack_.empty()) {
        // Корневой контекст - не массив
        return false;
    }
    return nodes_stack_.back()->IsArray();
}

bool Builder::IsComplete() const {
    return nodes_stack_.empty() && !root_.IsNull();
}

void Builder::AddValue(Node value) {
    if (IsComplete()) {
        throw std::logic_error("Builder is already complete");
    }
    
    if (nodes_stack_.empty()) {
        // Корневое значение
        root_ = std::move(value);
        return;
    }
    
    Node* current = nodes_stack_.back();
    
    if (current->IsArray()) {
        // Добавляем в массив
        auto& array = const_cast<Array&>(current->AsArray());
        array.push_back(std::move(value));
    } else if (current->IsMap() && expecting_value_) {
        // Добавляем в словарь по ключу
        auto& dict = const_cast<Dict&>(current->AsMap());
        dict[keys_.back()] = std::move(value);
        keys_.pop_back();
        expecting_value_ = false;
    } else {
        throw std::logic_error("Value can't be added in current context");
    }
}

Builder::DictItemContext Builder::StartDict() {
    if (IsComplete()) {
        throw std::logic_error("Builder is already complete");
    }
    
    Node dict_node{Dict{}};
    
    if (nodes_stack_.empty()) {
        // Корневой словарь
        root_ = std::move(dict_node);
        nodes_stack_.push_back(&root_);
    } else {
        Node* current = GetCurrentNode();
        
        if (current->IsArray()) {
            // Добавляем словарь в массив
            auto& array = const_cast<Array&>(current->AsArray());
            array.push_back(std::move(dict_node));
            nodes_stack_.push_back(&array.back());
        } else if (current->IsMap() && expecting_value_) {
            // Добавляем словарь как значение для ключа
            auto& dict = const_cast<Dict&>(current->AsMap());
            dict[keys_.back()] = std::move(dict_node);
            nodes_stack_.push_back(&dict[keys_.back()]);
            keys_.pop_back();
            expecting_value_ = false;
        } else {
            throw std::logic_error("StartDict can't be called in current context");
        }
    }
    
    return DictItemContext(*this);
}

Builder::ArrayItemContext Builder::StartArray() {
    if (IsComplete()) {
        throw std::logic_error("Builder is already complete");
    }
    
    Node array_node{Array{}};
    
    if (nodes_stack_.empty()) {
        // Корневой массив
        root_ = std::move(array_node);
        nodes_stack_.push_back(&root_);
    } else {
        Node* current = GetCurrentNode();
        
        if (current->IsArray()) {
            // Добавляем массив в массив
            auto& array = const_cast<Array&>(current->AsArray());
            array.push_back(std::move(array_node));
            nodes_stack_.push_back(&array.back());
        } else if (current->IsMap() && expecting_value_) {
            // Добавляем массив как значение для ключа
            auto& dict = const_cast<Dict&>(current->AsMap());
            dict[keys_.back()] = std::move(array_node);
            nodes_stack_.push_back(&dict[keys_.back()]);
            keys_.pop_back();
            expecting_value_ = false;
        } else {
            throw std::logic_error("StartArray can't be called in current context");
        }
    }
    
    return ArrayItemContext(*this);
}

Builder& Builder::Value(Node value) {
    if (IsComplete()) {
        throw std::logic_error("Builder is already complete");
    }
    
    if (nodes_stack_.empty()) {
        // Корневое значение
        root_ = std::move(value);
        return *this;
    }
    
    Node* current = GetCurrentNode();
    
    if (current->IsArray()) {
        // Добавляем в массив
        auto& array = const_cast<Array&>(current->AsArray());
        array.push_back(std::move(value));
        return *this;
    } else if (current->IsMap() && expecting_value_) {
        // Добавляем в словарь по ключу
        auto& dict = const_cast<Dict&>(current->AsMap());
        dict[keys_.back()] = std::move(value);
        keys_.pop_back();
        expecting_value_ = false;
        return *this;
    }
    
    throw std::logic_error("Value can't be added in current context");
}

Builder::DictKeyContext Builder::Key(std::string key) {
    if (IsComplete()) {
        throw std::logic_error("Builder is already complete");
    }
    
    if (!IsDictContext() || expecting_value_) {
        throw std::logic_error("Key can only be called in dict context without pending value");
    }
    
    keys_.push_back(std::move(key));
    expecting_value_ = true;
    
    return DictKeyContext(*this);
}

Builder& Builder::EndDict() {
    if (IsComplete()) {
        throw std::logic_error("Builder is already complete");
    }
    
    if (!IsDictContext() || expecting_value_) {
        throw std::logic_error("EndDict can only be called in dict context without pending key");
    }
    
    if (!nodes_stack_.empty()) {
        nodes_stack_.pop_back();
    }
    
    return *this;
}

Builder& Builder::EndArray() {
    if (IsComplete()) {
        throw std::logic_error("Builder is already complete");
    }
    
    if (!IsArrayContext()) {
        throw std::logic_error("EndArray can only be called in array context");
    }
    
    if (!nodes_stack_.empty()) {
        nodes_stack_.pop_back();
    }
    
    return *this;
}

Node Builder::Build() {
    if (!IsComplete()) {
        throw std::logic_error("JSON document is not complete");
    }
    
    if (root_.IsNull()) {
        throw std::logic_error("Empty JSON document");
    }
    
    return std::move(root_);
}

// Реализация методов контекстных классов

Builder::DictKeyContext Builder::DictItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& Builder::DictItemContext::EndDict() {
    return builder_.EndDict();
}

Builder::DictItemContext Builder::DictKeyContext::Value(Node value) {
    builder_.Value(std::move(value));
    return Builder::DictItemContext(builder_);
}

Builder::DictItemContext Builder::DictKeyContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::DictKeyContext::StartArray() {
    return builder_.StartArray();
}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node value) {
    builder_.Value(std::move(value));
    return Builder::ArrayItemContext(builder_);
}

Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& Builder::ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

} // namespace json
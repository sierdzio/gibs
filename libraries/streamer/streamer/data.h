#pragma once

//i library name streamer type static

#include <initializer_list>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

struct Object;
struct Array;

struct Value
{
    static void writeQuotedAndEscapedString(std::ostream &output, std::string_view value);

    using ObjectStorage = std::vector<std::pair<std::string, Value>>;
    using ArrayStorage = std::vector<Value>;
    using Storage = std::variant<std::monostate, std::string, int, double, bool,
                                 ObjectStorage, ArrayStorage>;

    Storage data;

    Value() = default;
    Value(const char *value);
    Value(std::string value);
    Value(int value);
    Value(double value);
    Value(bool value);
    Value(const Object &value);
    Value(Object &&value);
    Value(const Array &value);
    Value(Array &&value);

    const std::type_info &type() const;
};

struct Object
{
    Object() = default;
    Object(std::initializer_list<std::pair<const std::string, Value>> initialValues);

    void insert(const std::string &key, const Value &value);
    void insert(const std::string &key, Value &&value);

    std::unordered_map<std::string, Value> elements;
};

struct Array
{
    Array() = default;
    Array(std::initializer_list<Value> initialValues);

    void append(const Value &value);
    void append(Value &&value);

    std::vector<Value> elements;
};

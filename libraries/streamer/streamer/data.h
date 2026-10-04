#pragma once

//i library name streamer type static

#include <map>
#include <string>
#include <typeinfo>
#include <variant>
#include <vector>

struct Object;
struct Array;

struct Value
{
    static void writeQuotedAndEscapedString(std::ostream &output, std::string_view value);

    using ObjectStorage = std::map<std::string, Value>;
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

    const std::string &toString() const;

    const ObjectStorage &asObject() const;
};

struct Object : public std::map<std::string, Value>
{
    using std::map<std::string, Value>::map;

    void insert(const std::string &key, const Value &value);
    void insert(const std::string &key, Value &&value);
};

struct Array : public std::vector<Value>
{
    using std::vector<Value>::vector;

    void append(const Value &value);
    void append(Value &&value);
};

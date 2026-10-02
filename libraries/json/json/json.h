#pragma once

//i library name json type static

#include <filesystem>
#include <initializer_list>
#include <ostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

struct Object;
struct Array;

struct Value
{
    using ObjectStorage = std::vector<std::pair<std::string, Value>>;
    using ArrayStorage = std::vector<Value>;
    using Storage =
        std::variant<std::monostate, std::string, int, double, bool, ObjectStorage, ArrayStorage>;

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
    std::string toString() const;
};

struct Element
{
  protected:
    virtual std::string beginning() const;
    virtual std::string ending() const;
};

struct Object : public Element
{
    static std::string serialize(const Object &object);
    static void serialize(const Object &object, std::ostream &output);

    Object() = default;
    Object(std::initializer_list<std::pair<const std::string, Value>> initialValues);

    void insert(const std::string &key, const Value &value);
    void insert(const std::string &key, Value &&value);

    std::unordered_map<std::string, Value> elements;

  protected:
    std::string beginning() const final;
    std::string ending() const final;
};

struct Array : public Element
{
    static std::string serialize(const Array &array);
    static void serialize(const Array &array, std::ostream &output);

    Array() = default;
    Array(std::initializer_list<Value> initialValues);

    void append(const Value &value);
    void append(Value &&value);

    std::vector<Value> elements;

  protected:
    std::string beginning() const final;
    std::string ending() const final;
};

/*!
Super simple JSON library for C++.

Does not support much, but it is only intended to be used inside of gibs.
*/
template <typename Type>
    requires(std::is_base_of_v<Object, Type> || std::is_base_of_v<Array, Type>)
class Json : public Type
{
  public:
    bool write(const std::filesystem::path &path) const;
};

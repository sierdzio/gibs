#include "data.h"

#include <iostream>
#include <string_view>
#include <utility>

namespace
{
constexpr char FirstPrintableCharacter = 0x20;
constexpr char Hex[] = "0123456789abcdef";
} //namespace

void Value::writeQuotedAndEscapedString(std::ostream &output, std::string_view value)
{
    output.put('"');
    for (const char rawCharacter : value)
    {
        const auto character = static_cast<unsigned char>(rawCharacter);
        switch (character)
        {
        case '"':
            output << "\\\"";
            break;
        case '\\':
            output << "\\\\";
            break;
        case '\b':
            output << "\\b";
            break;
        case '\f':
            output << "\\f";
            break;
        case '\n':
            output << "\\n";
            break;
        case '\r':
            output << "\\r";
            break;
        case '\t':
            output << "\\t";
            break;
        default:
            if (character < FirstPrintableCharacter)
            {
                output << "\\u00";
                output.put(Hex[character >> 4]);
                output.put(Hex[character & 0x0f]);
            }
            else
            {
                output.put(static_cast<char>(character));
            }
        }
    }
    output.put('"');
}

Value::Value(const char *value) : data(std::string(value))
{
}

Value::Value(std::string value) : data(std::move(value))
{
}

Value::Value(int value) : data(value)
{
}

Value::Value(double value) : data(value)
{
}

Value::Value(bool value) : data(value)
{
}

Value::Value(const Object &value) : data(static_cast<const ObjectStorage &>(value))
{
}

Value::Value(Object &&value) : data(std::move(static_cast<ObjectStorage &>(value)))
{
}

Value::Value(const Array &value) : data(value)
{
}

Value::Value(Array &&value) : data(std::move(value))
{
}

const std::type_info &Value::type() const
{
    if (std::holds_alternative<std::string>(data))
    {
        return typeid(std::string);
    }
    if (std::holds_alternative<int>(data))
    {
        return typeid(int);
    }
    if (std::holds_alternative<double>(data))
    {
        return typeid(double);
    }
    if (std::holds_alternative<bool>(data))
    {
        return typeid(bool);
    }
    if (std::holds_alternative<ObjectStorage>(data))
    {
        return typeid(Object);
    }
    if (std::holds_alternative<ArrayStorage>(data))
    {
        return typeid(Array);
    }
    return typeid(std::monostate);
}

const std::string &Value::toString() const
{
    return std::get<std::string>(data);
}

const Value::ObjectStorage &Value::asObject() const
{
    return std::get<ObjectStorage>(data);
}

void Object::insert(const std::string &key, const Value &value)
{
    try_emplace(key, value);
}

void Object::insert(const std::string &key, Value &&value)
{
    try_emplace(key, std::move(value));
}

void Array::append(const Value &value)
{
    push_back(value);
}

void Array::append(Value &&value)
{
    push_back(std::move(value));
}

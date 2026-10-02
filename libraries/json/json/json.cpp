#include "json.h"

#include <fstream>
#include <sstream>
#include <string_view>

namespace
{
constexpr char FirstPrintableCharacter = 0x20;
constexpr char Hex[] = "0123456789abcdef";

void writeQuotedAndEscapedString(std::ostream &output, std::string_view value)
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

void writeValue(std::ostream &output, const Value &value, bool addTrailingNewline = true)
{
    if (std::holds_alternative<std::string>(value.data))
    {
        writeQuotedAndEscapedString(output, std::get<std::string>(value.data));
        return;
    }

    if (std::holds_alternative<int>(value.data))
    {
        output << std::get<int>(value.data);
        return;
    }

    if (std::holds_alternative<double>(value.data))
    {
        output << std::to_string(std::get<double>(value.data));
        return;
    }

    if (std::holds_alternative<bool>(value.data))
    {
        output << (std::get<bool>(value.data) ? "true" : "false");
        return;
    }

    if (std::holds_alternative<Value::ObjectStorage>(value.data))
    {
        const auto &elements = std::get<Value::ObjectStorage>(value.data);
        output << "{\n";
        for (size_t index = 0; index < elements.size(); ++index)
        {
            writeQuotedAndEscapedString(output, elements[index].first);
            output << ": ";
            writeValue(output, elements[index].second, false);
            if (index + 1 != elements.size())
            {
                output.put(',');
            }
            output.put('\n');
        }
        output.put('}');
        if (addTrailingNewline)
        {
            output.put('\n');
        }
        return;
    }

    if (std::holds_alternative<Value::ArrayStorage>(value.data))
    {
        const auto &elements = std::get<Value::ArrayStorage>(value.data);
        output.put('[');
        for (size_t index = 0; index < elements.size(); ++index)
        {
            writeValue(output, elements[index], false);
            if (index + 1 != elements.size())
            {
                output.put(',');
            }
        }
        output.put(']');
        if (addTrailingNewline)
        {
            output.put('\n');
        }
        return;
    }
}
} // namespace

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

Value::Value(const Object &value)
{
    ObjectStorage elements;
    elements.reserve(value.elements.size());
    for (const auto &[key, child] : value.elements)
    {
        elements.emplace_back(key, child);
    }
    data = std::move(elements);
}

Value::Value(Object &&value)
{
    ObjectStorage elements;
    elements.reserve(value.elements.size());
    for (auto &[key, child] : value.elements)
    {
        elements.emplace_back(key, std::move(child));
    }
    data = std::move(elements);
}

Value::Value(const Array &value) : data(value.elements)
{
}

Value::Value(Array &&value) : data(std::move(value.elements))
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

std::string Value::toString() const
{
    std::ostringstream output;
    writeValue(output, *this);
    return output.str();
}

std::string Element::beginning() const
{
    return {};
}

std::string Element::ending() const
{
    return {};
}

void Object::insert(const std::string &key, const Value &value)
{
    elements.try_emplace(key, value);
}

void Object::insert(const std::string &key, Value &&value)
{
    elements.try_emplace(key, std::move(value));
}

std::string Object::serialize(const Object &object)
{
    std::ostringstream output;
    serialize(object, output);
    return output.str();
}

void Object::serialize(const Object &object, std::ostream &output)
{
    output << object.beginning() << '\n';
    const auto sizeToCheck = object.elements.size() - 1;
    size_t index = 0;
    for (const auto &[key, value] : object.elements)
    {
        writeQuotedAndEscapedString(output, key);
        output << ": ";
        writeValue(output, value, false);
        if (index != sizeToCheck)
        {
            output.put(',');
        }
        output.put('\n');
        ++index;
    }
    output << object.ending() << '\n';
}

Object::Object(std::initializer_list<std::pair<const std::string, Value>> initialValues)
    : elements(initialValues)
{
}

std::string Object::beginning() const
{
    return "{";
}

std::string Object::ending() const
{
    return "}";
}

std::string Array::serialize(const Array &array)
{
    std::ostringstream output;
    serialize(array, output);
    return output.str();
}

void Array::serialize(const Array &array, std::ostream &output)
{
    output << array.beginning();
    const auto sizeToCheck = array.elements.size() - 1;
    for (size_t index = 0; index < array.elements.size(); ++index)
    {
        writeValue(output, array.elements[index], false);
        if (index != sizeToCheck)
        {
            output.put(',');
        }
    }
    output << array.ending() << '\n';
}

Array::Array(std::initializer_list<Value> initialValues) : elements(initialValues)
{
}

void Array::append(const Value &value)
{
    elements.push_back(value);
}

void Array::append(Value &&value)
{
    elements.push_back(std::move(value));
}

std::string Array::beginning() const
{
    return "[";
}

std::string Array::ending() const
{
    return "]";
}

template <typename Type>
    requires(std::is_base_of_v<Object, Type> || std::is_base_of_v<Array, Type>)
bool Json<Type>::write(const std::filesystem::path &path) const
{
    std::ofstream output(path);
    if (not output)
    {
        return false;
    }

    output << this->beginning() << "\n";

    const auto sizeToCheck = this->elements.size() - 1;
    size_t index = 0;
    for (const auto &current : this->elements)
    {
        if constexpr (std::is_base_of_v<Array, Type>)
        {
            writeValue(output, current, false);
        }
        else if constexpr (std::is_base_of_v<Object, Type>)
        {
            writeQuotedAndEscapedString(output, current.first);
            output << ": ";
            writeValue(output, current.second, false);
        }

        if (index != sizeToCheck)
        {
            output.put(',');
        }
        output.put('\n');
        ++index;
    }

    output << this->ending() << "\n";

    return output.good();
}

template class Json<Object>;
template class Json<Array>;

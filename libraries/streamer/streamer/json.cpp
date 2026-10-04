#include "json.h"

#include <fstream>
#include <sstream>

namespace
{
void writeValueInternal(std::ostream &output, const Value &value,
                        bool addTrailingNewline = true)
{
    if (std::holds_alternative<std::string>(value.data))
    {
        Value::writeQuotedAndEscapedString(output, std::get<std::string>(value.data));
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
            Value::writeQuotedAndEscapedString(output, elements[index].first);
            output << ": ";
            writeValueInternal(output, elements[index].second, false);
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
            writeValueInternal(output, elements[index], false);
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

template <typename Type>
    requires(std::is_base_of_v<Object, Type> || std::is_base_of_v<Array, Type>)
void Json<Type>::writeValue(std::ostream &output, const Value &value,
                            bool addTrailingNewline)
{
    writeValueInternal(output, value, addTrailingNewline);
}

template <typename Type>
    requires(std::is_base_of_v<Object, Type> || std::is_base_of_v<Array, Type>)
std::string Json<Type>::serialize(const Object &object)
{
    std::ostringstream output;
    serialize(object, output);
    return output.str();
}

template <typename Type>
    requires(std::is_base_of_v<Object, Type> || std::is_base_of_v<Array, Type>)
void Json<Type>::serialize(const Object &object, std::ostream &output)
{
    output << "{\n";
    const auto sizeToCheck = object.size() - 1;
    size_t index = 0;
    for (const auto &[key, value] : object)
    {
        Value::writeQuotedAndEscapedString(output, key);
        output << ": ";
        writeValue(output, value, false);
        if (index != sizeToCheck)
        {
            output.put(',');
        }
        output.put('\n');
        ++index;
    }
    output << "}\n";
}

template <typename Type>
    requires(std::is_base_of_v<Object, Type> || std::is_base_of_v<Array, Type>)
std::string Json<Type>::serialize(const Array &array)
{
    std::ostringstream output;
    serialize(array, output);
    return output.str();
}

template <typename Type>
    requires(std::is_base_of_v<Object, Type> || std::is_base_of_v<Array, Type>)
void Json<Type>::serialize(const Array &array, std::ostream &output)
{
    output << "[";
    const auto sizeToCheck = array.size() - 1;
    for (size_t index = 0; index < array.size(); ++index)
    {
        writeValue(output, array[index], false);
        if (index != sizeToCheck)
        {
            output.put(',');
        }
    }
    output << "]\n";
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

    if constexpr (std::is_base_of_v<Array, Type>)
    {
        output << "[\n";
    }
    else if constexpr (std::is_base_of_v<Object, Type>)
    {
        output << "{\n";
    }

    const auto sizeToCheck = this->size() - 1;
    size_t index = 0;
    for (const auto &current : (*this))
    {
        if constexpr (std::is_base_of_v<Array, Type>)
        {
            writeValue(output, current, false);
        }
        else if constexpr (std::is_base_of_v<Object, Type>)
        {
            Value::writeQuotedAndEscapedString(output, current.first);
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

    if constexpr (std::is_base_of_v<Array, Type>)
    {
        output << "]\n";
    }
    else if constexpr (std::is_base_of_v<Object, Type>)
    {
        output << "}\n";
    }

    return output.good();
}

template class Json<Object>;
template class Json<Array>;

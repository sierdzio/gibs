#include "ini.h"

#include <fstream>
#include <sstream>

namespace
{
constexpr auto Nl = '\n';
constexpr auto Eq = " = ";

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
        output << Nl;
        for (size_t index = 0; index < elements.size(); ++index)
        {
            Value::writeQuotedAndEscapedString(output, elements[index].first);
            output << Eq;
            writeValueInternal(output, elements[index].second, false);
            output.put(Nl);
        }
        if (addTrailingNewline)
        {
            output.put(Nl);
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
            output.put(Nl);
        }
        return;
    }
}
} // namespace

template <typename Type>
    requires(std::is_base_of_v<Object, Type>)
void Ini<Type>::writeValue(std::ostream &output, const Value &value,
                           bool addTrailingNewline)
{
    writeValueInternal(output, value, addTrailingNewline);
}

template <typename Type>
    requires(std::is_base_of_v<Object, Type>)
std::string Ini<Type>::serialize(const Object &object)
{
    std::ostringstream output;
    serialize(object, output);
    return output.str();
}

template <typename Type>
    requires(std::is_base_of_v<Object, Type>)
void Ini<Type>::serialize(const Object &object, std::ostream &output)
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
    requires(std::is_base_of_v<Object, Type>)
std::string Ini<Type>::serialize(const Array &array)
{
    std::ostringstream output;
    serialize(array, output);
    return output.str();
}

template <typename Type>
    requires(std::is_base_of_v<Object, Type>)
void Ini<Type>::serialize(const Array &array, std::ostream &output)
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
    requires(std::is_base_of_v<Object, Type>)
bool Ini<Type>::write(const std::filesystem::path &path) const
{
    std::ofstream output(path);
    if (not output)
    {
        return false;
    }

    if constexpr (std::is_base_of_v<Array, Type>)
    {
        // Top-level array - not supported in INI format
        // TODO: log
        return false;
    }

    for (const auto &current : (*this))
    {
        if constexpr (std::is_base_of_v<Object, Type>)
        {
            output << '[';
            Value::writeQuotedAndEscapedString(output, current.first);
            output << ']' << Nl;
            writeValue(output, current.second, false);
        }

        output.put(Nl);
    }

    return output.good();
}

template class Ini<Object>;

#include "ini.h"

#include <logger/log.h>

#include <cctype>
#include <cstdlib>
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
        for (const auto &[key, child] : elements)
        {
            Value::writeQuotedAndEscapedString(output, key);
            output << Eq;
            writeValueInternal(output, child, false);
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
        Log::error("INI format does not support top-level arrays");
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

namespace
{
std::string trim(std::string_view text)
{
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos)
    {
        return {};
    }

    const auto end = text.find_last_not_of(" \t\r\n");
    return std::string(text.substr(begin, end - begin + 1));
}

std::string unescapeQuotedString(std::string_view input)
{
    std::string result;
    result.reserve(input.size());

    for (size_t index = 0; index < input.size(); ++index)
    {
        const auto character = input[index];
        if (character != '\\' || index + 1 >= input.size())
        {
            result.push_back(character);
            continue;
        }

        const auto escape = input[index + 1];
        switch (escape)
        {
        case 'n':
            result.push_back('\n');
            break;
        case 'r':
            result.push_back('\r');
            break;
        case 't':
            result.push_back('\t');
            break;
        case 'b':
            result.push_back('\b');
            break;
        case 'f':
            result.push_back('\f');
            break;
        case '"':
            result.push_back('"');
            break;
        case '\\':
            result.push_back('\\');
            break;
        default:
            result.push_back(escape);
            break;
        }

        ++index;
    }

    return result;
}

Value parseValue(std::string_view input, size_t &position)
{
    while (position < input.size() && std::isspace(static_cast<unsigned char>(input[position])))
    {
        ++position;
    }

    if (position >= input.size())
    {
        return {};
    }

    if (input[position] == '"')
    {
        ++position;
        std::string value;
        while (position < input.size())
        {
            const auto character = input[position++];
            if (character == '\\')
            {
                if (position >= input.size())
                {
                    break;
                }

                const auto escaped = input[position++];
                switch (escaped)
                {
                case 'n':
                    value.push_back('\n');
                    break;
                case 'r':
                    value.push_back('\r');
                    break;
                case 't':
                    value.push_back('\t');
                    break;
                case 'b':
                    value.push_back('\b');
                    break;
                case 'f':
                    value.push_back('\f');
                    break;
                case '"':
                    value.push_back('"');
                    break;
                case '\\':
                    value.push_back('\\');
                    break;
                default:
                    value.push_back(escaped);
                    break;
                }
                continue;
            }

            if (character == '"')
            {
                break;
            }

            value.push_back(character);
        }

        while (position < input.size() && std::isspace(static_cast<unsigned char>(input[position])) &&
               input[position] != '\n' && input[position] != '\r')
        {
            ++position;
        }

        return Value{std::move(value)};
    }

    const auto start = position;
    while (position < input.size())
    {
        const auto current = input[position];
        if (std::isspace(static_cast<unsigned char>(current)) || current == '\n' ||
            current == '\r')
        {
            break;
        }
        ++position;
    }

    const auto raw = std::string(input.substr(start, position - start));
    if (raw == "true")
    {
        return Value{true};
    }
    if (raw == "false")
    {
        return Value{false};
    }

    const auto asInt = [](
        const std::string &text, bool &ok) -> int
    {
        const auto *start = text.c_str();
        char *end = nullptr;
        const long result = std::strtol(start, &end, 10);
        ok = end != start && *end == '\0';
        return static_cast<int>(result);
    };

    const auto asDouble = [](
        const std::string &text, bool &ok) -> double
    {
        const auto *start = text.c_str();
        char *end = nullptr;
        const double result = std::strtod(start, &end);
        ok = end != start && *end == '\0';
        return result;
    };

    bool integerOk = false;
    const auto integerValue = asInt(raw, integerOk);
    if (integerOk)
    {
        return Value{integerValue};
    }

    bool doubleOk = false;
    const auto doubleValue = asDouble(raw, doubleOk);
    if (doubleOk)
    {
        return Value{doubleValue};
    }

    return Value{raw};
}

std::string parseSectionName(std::string_view input)
{
    const auto begin = input.find('[');
    if (begin == std::string_view::npos)
    {
        return {};
    }

    const auto end = input.find_last_of(']');
    if (end == std::string_view::npos || end <= begin)
    {
        return {};
    }

    const auto raw = trim(input.substr(begin + 1, end - begin - 1));
    if (raw.empty())
    {
        return {};
    }

    if (raw.front() == '"' && raw.back() == '"')
    {
        return unescapeQuotedString(raw.substr(1, raw.size() - 2));
    }

    return std::string(raw);
}
} // namespace

template <typename Type>
    requires(std::is_base_of_v<Object, Type>)
bool Ini<Type>::read(const std::filesystem::path &path)
{
    std::ifstream input(path);
    if (not input)
    {
        Log::error("Failed to open INI file for reading:", path.string());
        return false;
    }

    this->clear();

    std::string currentSection;
    Object currentObject;
    std::string line;

    while (std::getline(input, line))
    {
        const auto trimmed = trim(line);
        if (trimmed.empty())
        {
            continue;
        }

        if (trimmed.front() == '[')
        {
            if (not currentSection.empty())
            {
                this->insert(currentSection, Value{std::move(currentObject)});
                currentObject.clear();
            }

            const auto sectionName = parseSectionName(trimmed);
            if (sectionName.empty())
            {
                Log::error("Malformed INI section header:", trimmed);
                return false;
            }

            currentSection = sectionName;
            continue;
        }

        const auto equals = trimmed.find('=');
        if (equals == std::string::npos)
        {
            Log::error("Malformed INI entry:", trimmed);
            return false;
        }

        const auto keyCandidate = trim(trimmed.substr(0, equals));
        if (keyCandidate.empty())
        {
            Log::error("Missing INI key in:", trimmed);
            return false;
        }

        std::string key = keyCandidate;
        if (key.front() == '"' && key.back() == '"')
        {
            key = unescapeQuotedString(key.substr(1, key.size() - 2));
        }

        size_t valuePosition = equals + 1;
        const auto value = parseValue(trimmed, valuePosition);
        if (currentSection.empty())
        {
            this->insert(key, value);
        }
        else
        {
            currentObject.insert(key, value);
        }
    }

    if (not currentSection.empty())
    {
        this->insert(currentSection, Value{std::move(currentObject)});
    }

    return true;
}

template class Ini<Object>;

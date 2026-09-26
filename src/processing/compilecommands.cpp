#include "compilecommands.h"

#include <fstream>
#include <string_view>

namespace
{
void writeJsonString(std::ostream &output, std::string_view value)
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
            if (character < 0x20)
            {
                constexpr char Hex[] = "0123456789abcdef";
                output << "\\u00" << Hex[character >> 4] << Hex[character & 0x0f];
            }
            else
            {
                output.put(static_cast<char>(character));
            }
        }
    }
    output.put('"');
}
} // namespace

void CompileCommands::add(const std::filesystem::path &directory,
                          const std::filesystem::path &file, const CommandData &command)
{
    _commands.push_back({directory, file, command});
}

bool CompileCommands::write(const std::filesystem::path &path) const
{
    std::ofstream output(path);
    if (not output)
    {
        return false;
    }

    output << "[\n";
    for (std::size_t index = 0; index < _commands.size(); ++index)
    {
        const auto &current = _commands.at(index);
        output << "  {\n    \"directory\": ";
        writeJsonString(output, current.directory.string());
        output << ",\n    \"arguments\": [";
        writeJsonString(output, current.command.command);
        for (const auto &argument : current.command.arguments)
        {
            output << ", ";
            writeJsonString(output, argument);
        }
        output << "],\n    \"file\": ";
        writeJsonString(output, current.file.string());
        output << "\n  }";
        if (index + 1 < _commands.size())
        {
            output.put(',');
        }
        output.put('\n');
    }
    output << "]\n";
    return output.good();
}
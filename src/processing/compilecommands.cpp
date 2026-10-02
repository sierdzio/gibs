#include "compilecommands.h"

#include <algorithm>
#include <iterator>
#include <utility>

void CompileCommands::add(const std::filesystem::path &directory,
                          const std::filesystem::path &file, const CommandData &command)
{
    Array arguments{Value{command.command}};
    for (const auto &argument : command.arguments)
    {
        arguments.append(Value{argument});
    }

    Object entry{{"directory", Value{directory.string()}},
                 {"arguments", Value{std::move(arguments)}},
                 {"file", Value{file.string()}}};

    const auto outputArgument = std::find(command.arguments.begin(), command.arguments.end(), "-o");
    if (outputArgument != command.arguments.end() &&
        std::next(outputArgument) != command.arguments.end())
    {
        entry.insert("output", Value{*std::next(outputArgument)});
    }

    _json.append(Value{std::move(entry)});
}

bool CompileCommands::write(const std::filesystem::path &path) const
{
    return _json.write(path);
}

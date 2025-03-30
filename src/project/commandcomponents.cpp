#include "commandcomponents.h"

#include <filesystem>

bool ExecutableComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Executable and name.size() > 0;
}

bool LibraryComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Library and name.size() > 0;
}

bool ObjectComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Source and name.size() > 0;
}

bool IncludeComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Include and path.size() > 0;
}

bool IncludeComponent::isPathToFile() const
{
    // TODO: these checks and results should be cached!
    const std::filesystem::path rawPath(path);
    return std::filesystem::is_regular_file(rawPath);
}

std::string IncludeComponent::libraryDirPath() const
{
    // TODO: these checks and results should be cached!

    const std::filesystem::path rawPath(path);
    // TODO: ger absolute path or something...
    return rawPath.string();
}

std::string IncludeComponent::libraryName() const
{
    // TODO: these checks and results should be cached!

    const std::filesystem::path rawPath(path);

    if (isPathToFile())
    {
        return rawPath.parent_path().filename();
    }

    return rawPath.filename();
}

bool OptionComponent::isValid(const Syntax::Command type) const
{
    return (type == Syntax::Command::Feature or type == Syntax::Command::Option)
        and name.size() > 0;
}

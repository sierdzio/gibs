#include "commandcomponents.h"
#include "parsing/syntax.h"
#include "tools/tools.h"

#include <filesystem>

bool ExecutableComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Executable and not name.empty();
}

bool LibraryComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Library and not name.empty();
}

bool ObjectComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Source
           and not name.empty()
           and not source.empty();
}

bool IncludeComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Include and not path.empty();
}

std::string IncludeComponent::dirPath() const
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

    if (Tools::isPathToFile(path))
    {
        return rawPath.parent_path().filename();
    }

    return rawPath.filename();
}

bool OptionComponent::isValid(const Syntax::Command type) const
{
    return (type == Syntax::Command::Feature or type == Syntax::Command::Option) and
           name.size() > 0;
}

#include "commandcomponents.h"

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

bool OptionComponent::isValid(const Syntax::Command type) const
{
    return (type == Syntax::Command::Feature or type == Syntax::Command::Option)
        and name.size() > 0;
}

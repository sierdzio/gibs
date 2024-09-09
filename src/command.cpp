#include "command.h"
#include "syntax.h"

bool Command::isValid() const
{
    if (whole.empty()) {
        return false;
    }

    const auto &first = whole.at(0);

    const bool commandIsOk = first == Syntax::Command::Source
        || first == Syntax::Command::Target
        || first == Syntax::Command::Lib
        || first == Syntax::Command::Define
        || first == Syntax::Command::Include;

    // TODO: check modifiers
    // || first == Syntax::Command::Type
    // || first == Syntax::Command::App
    // || first == Syntax::Command::Static
    // || first == Syntax::Command::Dynamic

    return commandIsOk;
}

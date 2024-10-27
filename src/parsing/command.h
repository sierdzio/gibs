#pragma once

#include "syntax.h"

#include <string>
#include <vector>
#include <memory>

struct Command
{
    bool isValid() const;
    bool append(const std::string &part);

    /*!
     Returns the entire Command as text.

     \note This method constructs the text each time it is called. Use sparringly.
     */
    std::string whole() const;

    Syntax::Command command = Syntax::Command::Invalid;
    std::vector<std::string> modifiers;
    std::string value;

    // Children commands, for example compilation commands for objects under a single linker
    std::unique_ptr<Command> children;
    // Parent in compilation is the linker
    std::weak_ptr<Command> parent;

private:
    bool isValidCommand(const std::string &command) const;
};

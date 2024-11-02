#pragma once

#include "parsing/syntax.h"
#include "targetid.h"

#include <string>
#include <vector>
// #include <memory>

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

    // // Children commands, for example compilation commands for objects under a single linker
    // std::shared_ptr<Command> children;
    // // Parent in compilation is the linker
    // std::weak_ptr<Command> parent;

    TargetId targetId;

private:
    bool isValidCommand(const std::string &command) const;
};

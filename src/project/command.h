#pragma once

#include "parsing/syntax.h"

#include <string>
#include <vector>

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

private:
    bool isValidCommand(const std::string &command) const;
};

#pragma once

#include "parsing/syntax.h"
#include "targetid.h"

#include <string>
#include <vector>

struct Component
{
    virtual bool isValid(const Syntax::Command type) const = 0;
};

struct ExecutableComponent : public Component
{
    bool isValid(const Syntax::Command type) const final;

    std::string name;
    std::vector<std::string> objects;
};

struct Command
{
    bool isValid() const;
    bool append(const std::string &part);

    bool isReadyToExecute() const;
    void setIsReadyToExecute(const bool ready);

    /*!
     Returns the entire Command as text.

     \note This method constructs the text each time it is called. Use sparringly.
     */
    std::string whole() const;
    std::string value() const;

    // Composition: additional members used by some command types
    ExecutableComponent executable;

    // General members
    TargetId targetId;
    TargetId parentId;
    Syntax::Command command = Syntax::Command::Invalid;
    std::vector<std::string> modifiers;
    bool isReadyToExe = false;

private:
    bool isValidCommand(const std::string &command) const;
    bool supportsModifiers(const Syntax::Command command) const;
};

#pragma once

#include "parsing/syntax.h"
#include "targetid.h"
#include "commandcomponents.h"

#include <string>
#include <vector>

using CommandId = uint;

struct Command
{
    Command();
    //Command(const Command& other);

    CommandId id() const;

    bool isValid() const;
    bool append(const std::string &part);

    /*!
     * After calling append() to add data to the command, call this method
     * to do processing of all the modifiers.
     */
    void finalize();

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
    LibraryComponent library;
    ObjectComponent object;
    IncludeComponent include;
    OptionComponent option;

    // General members
    TargetId targetId;
    CommandId parentId;
    Syntax::Command type = Syntax::Command::Invalid;
    std::vector<std::string> modifiers;
    bool isReadyToExe = false;

private:
    bool isValidCommand(const std::string &command) const;
    bool supportsModifiers(const Syntax::Command command) const;

    const CommandId _id = 0;

    bool parsingFailed = false;
};

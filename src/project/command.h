#pragma once

#include "commandcomponents.h"
#include "parsing/syntax.h"
#include "targetid.h"

#include <string>
#include <vector>

using CommandId = uint;

class Command
{
  public:
    Command();

    CommandId id() const;

    bool isValid() const;
    bool append(const std::string &part);
    bool hasModifiers() const;

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

    /*!
     If this command contains any file paths, first one will be returned here.
     Otherwise, an empty string is returned.
    */
    std::string path() const;

    bool addLinkObject(const std::string &name);

    const ExecutableComponent &executable() const;
    void setExecutableName(const std::string &name);
    const LibraryComponent &library() const;
    const ObjectComponent &object() const;
    const IncludeComponent &include() const;
    const OptionComponent &option() const;

    // General members
    // const TargetId &targetId() const;
    // const CommandId &parentId() const;
    // const Syntax::Command &type() const;

    // General members
    TargetId targetId;
    CommandId parentId;
    Syntax::Command type = Syntax::Command::Invalid;

  private:
    bool isValidCommand(const std::string &command) const;
    bool supportsModifiers(const Syntax::Command command) const;

    const CommandId _id = 0;

    std::vector<std::string> _modifiers;

    // Composition: additional members used by some command types
    ExecutableComponent _executable;
    LibraryComponent _library;
    ObjectComponent _object;
    IncludeComponent _include;
    OptionComponent _option;

    bool _parsingFailed = false;
    bool _isReadyToExe = false;
};

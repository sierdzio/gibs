#pragma once

#include "project/command.h"
#include "targetid.h"

#include <future>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Processor;

class Project
{
  public:
    Project(std::shared_ptr<Processor> processor);

    bool addCommand(const Command &command);

    CommandId linkCommandIdFor(const TargetId &id) const;
    Command &commandRef(const CommandId id);

    void onParsingFinished();

    void logCommandTree() const;

    std::vector<Command> commands;
    TargetId id;

  private:
    void generateDepths();
    int depth(const Command &command) const;
    void logCommand(const int depth, const Command &command, std::string *string) const;

    std::unordered_map<CommandId, int> _commandDepths;
    std::unordered_map<CommandId, std::shared_future<void>> _commandCompletionFutures;
    std::shared_ptr<Processor> _processor;
};

#pragma once

#include "project/command.h"
#include "targetid.h"

#include <filesystem>
#include <future>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Processor;

class Project
{
  public:
    Project(std::shared_ptr<Processor> processor);

    bool addCommand(const Command &command);
    void addTestTarget(const CommandId id);
    void addTestRunner(const std::filesystem::path &path);

    CommandId linkCommandIdFor(const TargetId &id,
                               const std::filesystem::path &path) const;
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
    std::unordered_set<CommandId> _testTargets;
    std::vector<std::filesystem::path> _testRunners;
    std::shared_ptr<Processor> _processor;
};

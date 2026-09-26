#pragma once

#include "tool.h"

#include <filesystem>
#include <vector>

struct CompileCommand
{
    std::filesystem::path directory;
    std::filesystem::path file;
    CommandData command;
};

class CompileCommands
{
  public:
    void add(const std::filesystem::path &directory, const std::filesystem::path &file,
             const CommandData &command);
    bool write(const std::filesystem::path &path) const;

  private:
    std::vector<CompileCommand> _commands;
};
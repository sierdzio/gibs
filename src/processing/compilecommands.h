#pragma once

#include "tool.h"

//i include ../../libraries/json
#include <json/json.h>

#include <filesystem>

class CompileCommands
{
  public:
    void add(const std::filesystem::path &directory, const std::filesystem::path &file,
             const CommandData &command);
    bool write(const std::filesystem::path &path) const;

  private:
    Json<Array> _json;
};
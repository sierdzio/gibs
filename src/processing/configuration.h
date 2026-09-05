#pragma once

#include "project/commandcomponents.h"

#include <filesystem>
#include <string>

class ConfigurationGenerator
{
  public:
    static bool generate(const std::filesystem::path &input,
                         const std::filesystem::path &output,
                         const std::vector<ConfigurationReplacement> &replacements,
                         std::string *error = nullptr);
};

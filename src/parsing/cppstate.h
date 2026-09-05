#pragma once

#include "project/commandcomponents.h"
#include "project/targetid.h"

#include <filesystem>
#include <optional>

struct CppState
{
    std::filesystem::path currentFile;
    TargetId id;
    bool isCommentBlock = false;
    bool isProjectCommentBlock = false;
    bool shouldFinish = false;
    std::optional<ConfigurationComponent> configuration;
};

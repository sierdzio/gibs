#pragma once

#include "project/targetid.h"

#include <filesystem>

struct CppState
{
    std::filesystem::path currentFile;
    TargetId id;
    bool isCommentBlock = false;
    bool isProjectCommentBlock = false;
    bool shouldFinish = false;
};

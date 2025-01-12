#pragma once

#include "project/targetid.h"

#include <string>

struct CppState
{
    TargetId id;
    std::string currentFileBaseName;
    bool isCommentBlock = false;
    bool isProjectCommentBlock = false;
    bool shouldFinish = false;
};

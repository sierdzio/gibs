#pragma once

#include "project/targetid.h"

struct CppState
{
    TargetId id;
    bool isCommentBlock = false;
    bool isProjectCommentBlock = false;
    bool shouldFinish = false;
};

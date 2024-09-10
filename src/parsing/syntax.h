#pragma once

#include <array>
#include <string>
#include <algorithm>

#define COMMANDS \
X(Invalid, "") \
X(Source, "source") \
X(Target, "target") \
X(Lib, "lib") \
X(Define, "define") \
X(Include, "include")

namespace Syntax {
    namespace Extension {
        constexpr auto ProjectFile = ".gibs";
        constexpr auto CppFile1 = ".cpp";
        constexpr auto CppFile2 = ".cxx";
        constexpr auto Main = "main";
    };

    namespace Comment {
        constexpr auto OneLineProject = "//i";
        constexpr auto MultilineBeginProject = "/*i";
        constexpr auto Project = '#';

        constexpr auto OneLine = "//";
        constexpr auto MultilineBegin = "/*";
        constexpr auto MultilineEnd = "*/";
    };

    #define X(day, name) day,
    enum Command {
        COMMANDS
    };
    #undef X

    #define X(day, name) name,
    const std::array<std::string, 6> commandString = {
        COMMANDS
    };
    #undef X

    Command commandValue(const std::string &string);

    namespace Modifier {
        constexpr auto Type = "type";
        constexpr auto App = "app";
        constexpr auto Static = "static";
        constexpr auto Dynamic = "dynamic";
    };

    namespace CppKeywords {
        constexpr auto Class = "class";
        constexpr auto Struct = "struct";
        constexpr auto Include = "#include";
    };
};

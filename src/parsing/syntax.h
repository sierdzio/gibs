#pragma once

#include <array>
#include <string>

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
        constexpr auto HeaderFile1 = ".h";
        constexpr auto HeaderFile2 = ".hpp";
        constexpr auto HeaderFile3 = ".hxx";
        constexpr auto ObjectFile1 = ".o";
        constexpr auto ObjectFile2 = ".obj";
        constexpr auto Main = "main";
    };

    enum class FileType {
        Other,
        Project,
        Cpp,
        H,
        Object
    };

    namespace Comment {
        constexpr auto OneLineProject = "//i";
        constexpr auto MultilineBeginProject = "/*i";
        constexpr auto Project = '#';

        constexpr auto OneLine = "//";
        constexpr auto MultilineBegin = "/*";
        constexpr auto MultilineEnd = "*/";
    };

    #define X(key, name) key,
    enum class Command {
        COMMANDS
    };
    #undef X

    #define X(key, name) name,
    constexpr std::array commandStrings = {
        COMMANDS
    };
    #undef X

    const std::string commandString(const Command command);
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

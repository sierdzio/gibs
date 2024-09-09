#pragma once

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

    namespace Command {
        constexpr auto Source = "source";
        constexpr auto Target = "target";
        constexpr auto Lib = "lib";
        constexpr auto Define = "define";
        constexpr auto Include = "include";
    };

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

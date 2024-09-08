#pragma once

namespace Extension {
    constexpr auto ProjectFile = ".gibs";
    constexpr auto CppFile1 = ".cpp";
    constexpr auto CppFile2 = ".cxx";
    constexpr auto Main = "main";
};

namespace Comment {
    constexpr auto OneLine = "//i ";
    constexpr auto MultilineBegin = "/*i ";
    constexpr auto MultilineEnd = "*/";
    constexpr auto Gibs = '#';
};

namespace Command {
    constexpr auto Source = "source";
    constexpr auto Target = "target";
    constexpr auto Type = "type";
    constexpr auto App = "app";
    constexpr auto Lib = "lib";
    constexpr auto Static = "static";
    constexpr auto Dynamic = "dynamic";
    constexpr auto Define = "define";
    constexpr auto Include = "include";
};

#pragma once

#include <string>

#define COMMANDS                                                                         \
    X(Invalid, "")                                                                       \
    X(Source, "source")                                                                  \
    X(Library, "library")                                                                \
    X(Define, "define")                                                                  \
    X(Include, "include")                                                                \
    X(Executable, "executable")                                                          \
    X(Feature, "feature")                                                                \
    X(Option, "option")                                                                  \
    X(Subproject, "subproject")                                                          \
    X(Tool, "tool")                                                                      \
    X(Qt, "qt")

namespace Syntax
{
namespace Extension
{
constexpr auto ProjectFile = ".gibs";
constexpr auto CppFile1 = ".cpp";
constexpr auto CppFile2 = ".cxx";
constexpr auto HeaderFile1 = ".h";
constexpr auto HeaderFile2 = ".hpp";
constexpr auto HeaderFile3 = ".hxx";
constexpr auto ObjectFile1 = ".o";
constexpr auto ObjectFile2 = ".obj";
// TODO: make it platform-dependent!
constexpr auto LibraryStatic = ".a";
constexpr auto LibraryDynamic = ".so";
constexpr auto Main = "main";
}; // namespace Extension

enum class FileType
{
    Other,
    Project,
    Cpp,
    H,
    Object
};

namespace Comment
{
constexpr auto OneLineProject = "//i";
constexpr auto MultilineBeginProject = "/*i";
constexpr auto Project = '#';

constexpr auto OneLine = "//";
constexpr auto MultilineBegin = "/*";
constexpr auto MultilineEnd = "*/";
}; // namespace Comment

#define X(key, name) key,
enum class Command
{
    COMMANDS
};
#undef X

const std::string commandString(const Command command);
Command commandValue(const std::string &string);
size_t commandCount();

namespace Modifier
{
constexpr auto Type = "type";
constexpr auto App = "app";
constexpr auto Static = "static";
constexpr auto Dynamic = "dynamic";
constexpr auto Name = "name";
constexpr auto Library = "library";
constexpr auto Default = "default";
constexpr auto On = "on";
constexpr auto Off = "off";
}; // namespace Modifier

enum class LibraryType
{
    Dynamic,
    Static
};

namespace CppKeywords
{
constexpr auto Class = "class";
constexpr auto Struct = "struct";
constexpr auto Include = "#include";
constexpr auto Main = "main";
constexpr auto Int = "int";
constexpr auto Char = "char";
constexpr auto DoubleColon = "::";
constexpr auto RoundBrackets = "()";
constexpr auto OpenLibraryInclude = '<';
constexpr auto CloseLibraryInclude = '>';
}; // namespace CppKeywords
}; // namespace Syntax

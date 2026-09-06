#pragma once

#include <string_view>

#define COMMANDS                                                                         \
    X(Unknown, "unknown")                                                                \
    X(Invalid, "invalid")                                                                \
    X(Source, "source")                                                                  \
    X(Library, "library")                                                                \
    X(Define, "define")                                                                  \
    X(Include, "include")                                                                \
    X(Executable, "executable")                                                          \
    X(Feature, "feature")                                                                \
    X(Option, "option")                                                                  \
    X(Subproject, "subproject")                                                          \
    X(Tool, "tool")                                                                      \
    X(Qt, "qt")                                                                          \
    X(Configure, "configure")                                                            \
    X(Replace, "replace")

namespace Syntax
{
namespace Extension
{
constexpr std::string_view ProjectFile = ".gibs";
constexpr std::string_view CppFile1 = ".cpp";
constexpr std::string_view CppFile2 = ".cxx";
constexpr std::string_view HeaderFile1 = ".h";
constexpr std::string_view HeaderFile2 = ".hpp";
constexpr std::string_view HeaderFile3 = ".hxx";
constexpr std::string_view ObjectFile1 = ".o";
constexpr std::string_view ObjectFile2 = ".obj";
// TODO: make it platform-dependent!
constexpr std::string_view LibraryStatic = ".a";
constexpr std::string_view LibraryDynamic = ".so";
constexpr std::string_view Main = "main";
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
constexpr std::string_view OneLineProject = "//i";
constexpr std::string_view MultilineBeginProject = "/*i";
constexpr char Project = '#';

constexpr std::string_view OneLine = "//";
constexpr std::string_view MultilineBegin = "/*";
constexpr std::string_view MultilineEnd = "*/";
}; // namespace Comment

#define X(key, name) key,
enum class Command
{
    COMMANDS
};
#undef X

std::string_view commandString(const Command command);
Command commandValue(std::string_view string);
size_t commandCount();

namespace Modifier
{
constexpr std::string_view Type = "type";
constexpr std::string_view App = "app";
constexpr std::string_view Static = "static";
constexpr std::string_view Dynamic = "dynamic";
constexpr std::string_view Name = "name";
constexpr std::string_view Version = "version";
constexpr std::string_view Input = "input";
constexpr std::string_view Output = "output";
constexpr std::string_view Replace = "replace";
constexpr std::string_view With = "with";
constexpr std::string_view Library = "library";
constexpr std::string_view Default = "default";
constexpr std::string_view On = "on";
constexpr std::string_view Off = "off";
}; // namespace Modifier

enum class LibraryType
{
    Dynamic,
    Static
};

namespace CppKeywords
{
constexpr std::string_view Class = "class";
constexpr std::string_view Struct = "struct";
constexpr std::string_view Include = "#include";
constexpr std::string_view Main = "main";
constexpr std::string_view Int = "int";
constexpr std::string_view Char = "char";
constexpr std::string_view Namespace = "namespace";
constexpr std::string_view DoubleColon = "::";
constexpr std::string_view RoundBrackets = "()";
constexpr char OpenLibraryInclude = '<';
constexpr char CloseLibraryInclude = '>';
}; // namespace CppKeywords
}; // namespace Syntax

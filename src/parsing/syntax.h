#pragma once

#include <string_view>

// TODO: add examples to all descriptions. Also, add cross-references between commands.
// Also, mention and sync this with README
#define COMMANDS                                                                         \
    X(Unknown, "unknown", "Unknown command")                                             \
    X(Invalid, "invalid", "Invalid command")                                             \
    X(Source, "source", "Use it to point gibs to a specific source file (.cpp)")         \
    X(Library, "library", "Defines a library target")                                    \
    X(Define, "define", "Can be used to add a define to compiler calls.")                \
    X(Include, "include", "Used to add include paths and files.")                        \
    X(Executable, "executable", "Defines an executable target.")                         \
    X(Feature, "feature",                                                                \
      "Feature can be turned on or off when calling gibs. This in turn can trigger "     \
      "changes in the source code via changed defines.")                                 \
    X(Option, "option", "Synonym to Feature")                                            \
    X(Subproject, "subproject", "NOT IMPLEMENTED!")                                      \
    X(Tool, "tool",                                                                      \
      "Allows running external executables, for example: \n//i tool myexecutable.exe "   \
      "--some -a -r -g -s")                                                              \
    X(Qt, "qt", "NOT IMPLEMENTED! Load Qt modules")                                      \
    X(Configure, "configure",                                                            \
      "Configuration file can be used to generate a file based on a template. Built-in " \
      "functions are replaced without prompting. For "                                   \
      "example: \n//i configure file input something.h.in output something.h \n//i "     \
      "replace \"some-text\" with target.main.version() \n//i replace \"another_text\" " \
      "with \"hello, config file!\"")                                                    \
    X(Replace, "replace",                                                                \
      "Use in combination with configure command to specify which strings should be "    \
      "replaced.")

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

#define X(key, name, description) key,
enum class Command
{
    COMMANDS
};
#undef X

std::string_view commandString(const Command command);
std::string_view commandDescription(const Command command);
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

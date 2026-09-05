#include <algorithm>
#include <gtest/gtest.h>

#include "processing/compiler.h"
#include "processing/compilerset.h"
#include "processing/linker.h"
#include "project/command.h"

namespace
{
const Paths DefaultPaths = {std::filesystem::current_path(),
                            std::filesystem::current_path(),
                            {},
                            {},
                            std::filesystem::current_path() / "build",
                            {}};
} // namespace

TEST(processing, CompilerCommands)
{
    Command command;
    EXPECT_TRUE(command.append("source"));
    EXPECT_TRUE(command.append("main.cpp"));
    command.finalize({}, DefaultPaths);

    command.objectReference().includePaths = {"include"};

    Compiler tool(command, CompilerSet::fromName("gcc"));
    const auto &commands = tool.commands();

    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].command, "g++");
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "-I") != commands[0].arguments.end());
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "include") != commands[0].arguments.end());
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "-o") != commands[0].arguments.end());
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "main.o") != commands[0].arguments.end());
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "main.cpp") != commands[0].arguments.end());
}

TEST(processing, LinkerStaticLibraryMultiCommand)
{
    Command command;
    EXPECT_TRUE(command.append("library"));
    EXPECT_TRUE(command.append("type"));
    EXPECT_TRUE(command.append("static"));
    EXPECT_TRUE(command.append("name"));
    EXPECT_TRUE(command.append("mylib"));
    command.finalize({}, DefaultPaths);

    EXPECT_TRUE(command.addLinkObject("file1.o"));
    EXPECT_TRUE(command.addLinkObject("file2.o"));

    Linker tool(command);
    const auto &commands = tool.commands();

    ASSERT_EQ(commands.size(), 2u);

    EXPECT_EQ(commands[0].command, "ar");
    EXPECT_EQ(commands[1].command, "ranlib");

    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "qc") != commands[0].arguments.end());
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "mylib.a") != commands[0].arguments.end());
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "file1.o") != commands[0].arguments.end());
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "file2.o") != commands[0].arguments.end());

    EXPECT_EQ(commands[1].arguments.size(), 1u);
    EXPECT_EQ(commands[1].arguments[0], "mylib.a");
}
TEST(processing, CompilerDefines)
{
    Command command;
    EXPECT_TRUE(command.append("source"));
    EXPECT_TRUE(command.append("main.cpp"));
    command.finalize({}, DefaultPaths);

    // Add some defines
    command.objectReference().defines = {"DEBUG", "MY_FEATURE", "VERSION=1"};
    command.objectReference().includePaths = {"include"};

    Compiler tool(command, CompilerSet::fromName("gcc"));
    const auto &commands = tool.commands();

    ASSERT_EQ(commands.size(), 1u);

    // Check that defines are present as -D<value> arguments
    const auto &args = commands[0].arguments;

    EXPECT_TRUE(std::find(args.begin(), args.end(), "-DDEBUG") != args.end());
    EXPECT_TRUE(std::find(args.begin(), args.end(), "-DMY_FEATURE") != args.end());
    EXPECT_TRUE(std::find(args.begin(), args.end(), "-DVERSION=1") != args.end());
}

TEST(processing, CompilerUsesExplicitToolchain)
{
    Command command;
    EXPECT_TRUE(command.append("source"));
    EXPECT_TRUE(command.append("main.cpp"));
    command.finalize({}, DefaultPaths);

    Compiler tool(command, CompilerSet::fromName("clang"));
    const auto &commands = tool.commands();

    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].command, "clang++");
}

TEST(processing, LinkerUsesExplicitToolchain)
{
    Command command;
    EXPECT_TRUE(command.append("executable"));
    EXPECT_TRUE(command.append("main"));
    command.finalize({}, DefaultPaths);

    command.addLinkObject("main.o");

    Linker tool(command, CompilerSet::fromName("clang"));
    const auto &commands = tool.commands();

    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].command, "clang++");
}

TEST(processing, ExecutableLinkOutputUsesBuildDirectory)
{
    Command command;
    EXPECT_TRUE(command.append("executable"));
    EXPECT_TRUE(command.append("gibs"));
    command.finalize({}, DefaultPaths);
    command.addLinkObject("main.o");

    Linker tool(command);
    const auto &commands = tool.commands();

    ASSERT_EQ(commands.size(), 1u);
    const auto output =
        std::find(commands[0].arguments.begin(), commands[0].arguments.end(), "-o");
    ASSERT_NE(output, commands[0].arguments.end());
    ASSERT_NE(std::next(output), commands[0].arguments.end());
    EXPECT_EQ(*std::next(output),
              (DefaultPaths.buildDirectory / "gibs").lexically_normal().string());
}

TEST(processing, CompilerEmptyDefines)
{
    Command command;
    EXPECT_TRUE(command.append("source"));
    EXPECT_TRUE(command.append("test.cpp"));
    command.finalize({}, DefaultPaths);

    // Add includes with empty defines (to ensure empty defines are skipped)
    command.objectReference().defines = {"DEBUG", "", "MY_FEATURE"};

    Compiler tool(command, CompilerSet::fromName("gcc"));
    const auto &commands = tool.commands();

    ASSERT_EQ(commands.size(), 1u);

    const auto &args = commands[0].arguments;

    // Count defines emitted as -D<value> arguments.
    auto dCount = std::count_if(args.begin(), args.end(), [](const std::string &arg)
                                { return arg.rfind("-D", 0) == 0 && arg.size() > 2; });
    // Should be 2 (for DEBUG and MY_FEATURE), not 3
    EXPECT_EQ(dCount, 2);
}

TEST(processing, CompilerSetSequentialCommandsDefault)
{
    const auto set = CompilerSet::fromName("gcc");
    EXPECT_EQ(set.commandExecution, CompilerSet::CommandExecution::Sequential);
}
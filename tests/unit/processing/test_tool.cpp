#include <algorithm>
#include <gtest/gtest.h>

#include "parsing/syntax.h"
#include "processing/compiler.h"
#include "processing/linker.h"
#include "project/command.h"

TEST(processing, CompilerCommands)
{
    Command command;
    EXPECT_TRUE(command.append("source"));
    EXPECT_TRUE(command.append("main.cpp"));
    command.finalize();

    command.objectReference().includePaths = {"include"};

    Compiler tool(command);
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
    command.finalize();

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
    command.finalize();

    // Add some defines
    command.objectReference().defines = {"DEBUG", "MY_FEATURE", "VERSION=1"};
    command.objectReference().includePaths = {"include"};

    Compiler tool(command);
    const auto &commands = tool.commands();

    ASSERT_EQ(commands.size(), 1u);

    // Check that defines are present as -D flags
    const auto &args = commands[0].arguments;

    EXPECT_TRUE(std::find(args.begin(), args.end(), "-D") != args.end());
    EXPECT_TRUE(std::find(args.begin(), args.end(), "DEBUG") != args.end());
    EXPECT_TRUE(std::find(args.begin(), args.end(), "MY_FEATURE") != args.end());
    EXPECT_TRUE(std::find(args.begin(), args.end(), "VERSION=1") != args.end());
}

TEST(processing, CompilerEmptyDefines)
{
    Command command;
    EXPECT_TRUE(command.append("source"));
    EXPECT_TRUE(command.append("test.cpp"));
    command.finalize();

    // Add includes with empty defines (to ensure empty defines are skipped)
    command.objectReference().defines = {"DEBUG", "", "MY_FEATURE"};

    Compiler tool(command);
    const auto &commands = tool.commands();

    ASSERT_EQ(commands.size(), 1u);

    const auto &args = commands[0].arguments;

    // Count -D flags
    auto dCount = std::count(args.begin(), args.end(), "-D");
    // Should be 2 (for DEBUG and MY_FEATURE), not 3
    EXPECT_EQ(dCount, 2);
}
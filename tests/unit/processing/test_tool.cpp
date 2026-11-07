#include <algorithm>
#include <gtest/gtest.h>

#include "processing/compilecommands.h"
#include "processing/compiler.h"
#include "processing/compilerset.h"
#include "processing/linker.h"
#include "project/command.h"

#include <fstream>
#include <iterator>

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
    EXPECT_TRUE(
        std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                  (DefaultPaths.buildDirectory / "main.o").lexically_normal().string()) !=
        commands[0].arguments.end());
    EXPECT_TRUE(
        std::find(
            commands[0].arguments.begin(), commands[0].arguments.end(),
            (DefaultPaths.workingDirectory / "main.cpp").lexically_normal().string()) !=
        commands[0].arguments.end());
}

TEST(processing, CompileCommandsWritesStandardDatabaseEntry)
{
    const auto path =
        std::filesystem::temp_directory_path() / "gibs_compile_commands_unit_test.json";
    CommandData command;
    command.command = "c++";
    command.arguments = {"-DNAME=\"value\"", "-o", "build object.o", "source file.cpp"};

    CompileCommands compileCommands;
    compileCommands.add("/project/build", "/project/source file.cpp", command);
    EXPECT_TRUE(compileCommands.write(path));

    std::ifstream input(path);
    const std::string contents((std::istreambuf_iterator<char>(input)),
                               std::istreambuf_iterator<char>());
    std::filesystem::remove(path);

    EXPECT_NE(contents.find("\"directory\": \"/project/build\""), std::string::npos);
    EXPECT_NE(contents.find("\"arguments\": [\"c++\",\"-DNAME=\\\"value\\\"\","
                            "\"-o\",\"build object.o\",\"source file.cpp\"]"),
              std::string::npos);
    EXPECT_NE(contents.find("\"file\": \"/project/source file.cpp\""), std::string::npos);
    EXPECT_NE(contents.find("\"output\": \"build object.o\""), std::string::npos);
    EXPECT_EQ(contents.find("\"command\""), std::string::npos);
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
    EXPECT_TRUE(
        std::find(
            commands[0].arguments.begin(), commands[0].arguments.end(),
            (DefaultPaths.buildDirectory / "mylib.a").lexically_normal().string()) !=
        commands[0].arguments.end());
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "file1.o") != commands[0].arguments.end());
    EXPECT_TRUE(std::find(commands[0].arguments.begin(), commands[0].arguments.end(),
                          "file2.o") != commands[0].arguments.end());

    EXPECT_EQ(commands[1].arguments.size(), 1u);
    EXPECT_EQ(commands[1].arguments[0],
              (DefaultPaths.buildDirectory / "mylib.a").lexically_normal().string());
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

TEST(processing, CompilerSetDefaultForPlatformUsesPlatformDefaults)
{
    const auto set = CompilerSet::defaultForPlatform();

#ifdef __APPLE__
    EXPECT_EQ(set.name, "apple-clang");
    EXPECT_EQ(set.compilerCommand, "clang++");
#else
    EXPECT_EQ(set.name, "gcc");
    EXPECT_EQ(set.compilerCommand, "g++");
#endif
    EXPECT_EQ(set.archiverCommand, "ar");
    EXPECT_EQ(set.ranlibCommand, "ranlib");
    EXPECT_EQ(set.commandExecution, CompilerSet::CommandExecution::Sequential);
}

TEST(processing, CompilerSetBuiltInNamesAreCaseInsensitive)
{
    EXPECT_EQ(CompilerSet::fromName("gcc").name, "gcc");
    EXPECT_EQ(CompilerSet::fromName("GCC").name, "gcc");
    EXPECT_EQ(CompilerSet::fromName("clang").name, "clang");
    EXPECT_EQ(CompilerSet::fromName("ClAng").name, "clang");
    EXPECT_EQ(CompilerSet::fromName("apple-clang").name, "apple-clang");
    EXPECT_EQ(CompilerSet::fromName("APPLE-CLANG").name, "apple-clang");
}

TEST(processing, CompilerSetKnownNamesAndFallbacks)
{
    EXPECT_TRUE(CompilerSet::isKnownName("gcc"));
    EXPECT_TRUE(CompilerSet::isKnownName("clang"));
    EXPECT_TRUE(CompilerSet::isKnownName("apple-clang"));
    EXPECT_FALSE(CompilerSet::isKnownName("unknown"));

    const auto set = CompilerSet::fromName("missing-compiler-set.ini");
    const auto defaultSet = CompilerSet::defaultForPlatform();
    EXPECT_EQ(set.name, defaultSet.name);
    EXPECT_EQ(set.compilerCommand, defaultSet.compilerCommand);
    EXPECT_EQ(set.archiverCommand, defaultSet.archiverCommand);
    EXPECT_EQ(set.ranlibCommand, defaultSet.ranlibCommand);
    EXPECT_EQ(set.commandExecution, defaultSet.commandExecution);
}

namespace
{
class CompilerSetIniTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        directory = std::filesystem::temp_directory_path() /
                   ("gibs-compilerset-test-" +
                    std::to_string(testing::UnitTest::GetInstance()->random_seed()));
        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);
    }

    void TearDown() override { std::filesystem::remove_all(directory); }

    std::filesystem::path directory;
};
} // namespace

TEST_F(CompilerSetIniTest, CompilerSetReadsCustomIniDefinitions)
{
    const auto path = directory / "custom-compiler-set.ini";

    std::ofstream output(path);
    output << "[compiler-set]\n";
    output << "name = custom\n";
    output << "compiler = custom-compiler\n";
    output << "archiver = custom-archiver\n";
    output << "ranlib = custom-ranlib\n";
    output << "execution = Parallel\n";
    output.close();

    const auto set = CompilerSet::fromName(path.string());

    EXPECT_EQ(set.name, "custom");
    EXPECT_EQ(set.compilerCommand, "custom-compiler");
    EXPECT_EQ(set.archiverCommand, "custom-archiver");
    EXPECT_EQ(set.ranlibCommand, "custom-ranlib");
    EXPECT_EQ(set.commandExecution, CompilerSet::CommandExecution::Parallel);
}

TEST(processing, CompilerSetSequentialCommandsDefault)
{
    const auto set = CompilerSet::fromName("gcc");
    EXPECT_EQ(set.commandExecution, CompilerSet::CommandExecution::Sequential);
}
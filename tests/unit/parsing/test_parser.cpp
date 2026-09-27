#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include <logger/log.h>
#include <parsing/parser.h>
#include <processing/processor.h>
#include <project/project.h>

TEST(parsing, ParserInheritsFeatureDefinesToCompileCommands)
{
    Log::setLogLevel(Log::Type::Silent);

    std::shared_ptr<Processor> processor = std::make_shared<Processor>();
    processor->setDryRun(true);
    auto project = std::make_shared<Project>(processor);

    const auto samplePath = std::filesystem::path(__FILE__).parent_path() /
                            std::filesystem::path("../../../samples/feature/main.cpp");

    ArgumentsList arguments;
    arguments["my-feature"] = true;
    Parser parser(samplePath, true, arguments, project);
    ASSERT_EQ(parser.status(), AppError::NoError);

    parser.parse();

    const auto buildDirectory = std::filesystem::current_path() / "build";
    bool compileCommandFound = false;
    bool mainObjectFound = false;
    for (const auto &command : project->commands)
    {
        if (command.type == Syntax::Command::Source)
        {
            compileCommandFound = true;
            EXPECT_TRUE(command.object().name.starts_with(buildDirectory.string() + "/"));
            if (command.object().sourcePath.filename() == "main.cpp")
            {
                mainObjectFound = true;
                EXPECT_EQ(command.object().name,
                          (buildDirectory / "main.o").lexically_normal());
            }
            const auto &defines = command.object().defines;
            EXPECT_NE(std::find(defines.begin(), defines.end(), "MY_FEATURE"),
                      defines.end());
        }
    }

    EXPECT_TRUE(compileCommandFound);
    EXPECT_TRUE(mainObjectFound);

    bool executableOutputFound = false;
    for (const auto &command : project->commands)
    {
        if (command.type == Syntax::Command::Executable)
        {
            executableOutputFound = true;
            EXPECT_EQ(command.executable().outputPath,
                      (buildDirectory / "SimpleTestFeature").lexically_normal());
        }
    }

    EXPECT_TRUE(executableOutputFound);
}

TEST(parsing, ParserReadsFinalProjectCommandToken)
{
    Log::setLogLevel(Log::Type::Silent);

    std::shared_ptr<Processor> processor = std::make_shared<Processor>();
    processor->setDryRun(true);
    auto project = std::make_shared<Project>(processor);

    const auto projectPath = std::filesystem::path(__FILE__).parent_path() /
                             std::filesystem::path("../../../main.gibs");

    Parser parser(projectPath, true, {}, project);
    ASSERT_EQ(parser.status(), AppError::NoError);

    parser.parse();

    bool sourceCommandFound = false;
    bool loggerIncludePathFound = false;
    for (const auto &command : project->commands)
    {
        if (command.type == Syntax::Command::Source)
        {
            sourceCommandFound = true;
            for (const auto &includePath : command.object().includePaths)
            {
                loggerIncludePathFound |= includePath.ends_with("libraries/logger");
            }
        }
    }

    EXPECT_TRUE(sourceCommandFound);
    EXPECT_TRUE(loggerIncludePathFound);
}

TEST(parsing, ParserSchedulesToolCommand)
{
    Log::setLogLevel(Log::Type::Silent);

    const auto directory = std::filesystem::temp_directory_path() / "gibs-tool-test";
    std::filesystem::create_directories(directory);
    const auto sourcePath = directory / "main.cpp";
    {
        std::ofstream source(sourcePath);
        source << "//i tool true --tool-argument\n";
        source << "int main() { return 0; }\n";
    }

    auto processor = std::make_shared<Processor>();
    processor->setDryRun(true);
    auto project = std::make_shared<Project>(processor);

    Parser parser(sourcePath, false, {}, project);
    ASSERT_EQ(parser.status(), AppError::NoError);
    parser.parse();

    const auto tool = std::find_if(project->commands.begin(), project->commands.end(),
                                   [](const Command &command)
                                   { return command.type == Syntax::Command::Tool; });
    ASSERT_NE(tool, project->commands.end());
    EXPECT_TRUE(tool->isReadyToExecute());
    EXPECT_EQ(tool->tool().executable, "true");
    ASSERT_EQ(tool->tool().arguments.size(), 1);
    EXPECT_EQ(tool->tool().arguments.front(), "--tool-argument");

    processor->waitForFinished();
    std::filesystem::remove_all(directory);
}

TEST(parsing, ParserReportsInvalidProjectCommand)
{
    Log::setLogLevel(Log::Type::Silent);

    const auto directory = std::filesystem::temp_directory_path() / "gibs-invalid-command-test";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    const auto sourcePath = directory / "main.cpp";
    {
        std::ofstream source(sourcePath);
        source << "//i target name InvalidCommandTest\n";
        source << "int main() { return 0; }\n";
    }

    auto processor = std::make_shared<Processor>();
    processor->setDryRun(true);
    auto project = std::make_shared<Project>(processor);
    Parser parser(sourcePath, false, {}, project);

    ASSERT_EQ(parser.status(), AppError::NoError);
    parser.parse();

    EXPECT_EQ(parser.status(), AppError::InvalidProjectCommand);

    processor->waitForFinished();
    std::filesystem::remove_all(directory);
}

TEST(parsing, ParserReportsInvalidProjectFileCommand)
{
    Log::setLogLevel(Log::Type::Silent);

    const auto directory =
        std::filesystem::temp_directory_path() / "gibs-invalid-project-file-command-test";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    const auto projectPath = directory / "main.gibs";
    {
        std::ofstream projectFile(projectPath);
        projectFile << "target name InvalidCommandTest\n";
    }

    auto processor = std::make_shared<Processor>();
    processor->setDryRun(true);
    auto project = std::make_shared<Project>(processor);
    Parser parser(projectPath, false, {}, project);

    ASSERT_EQ(parser.status(), AppError::NoError);
    parser.parse();

    EXPECT_EQ(parser.status(), AppError::InvalidProjectCommand);

    processor->waitForFinished();
    std::filesystem::remove_all(directory);
}

TEST(parsing, ParserReadsQuotedTestDirectory)
{
    Log::setLogLevel(Log::Type::Silent);

    const auto directory =
        std::filesystem::temp_directory_path() / "gibs parser test directory";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory / "test suite");
    {
        std::ofstream projectFile(directory / "main.gibs");
        projectFile << "source main.cpp\n";
        projectFile << "tests directory \"test suite\"\n";
        std::ofstream mainSource(directory / "main.cpp");
        mainSource << "int main() { return 0; }\n";
        std::ofstream testSource(directory / "test suite" / "main.cpp");
        testSource << "int main() { return 0; }\n";
    }

    auto processor = std::make_shared<Processor>();
    processor->setDryRun(true);
    auto project = std::make_shared<Project>(processor);
    project->setTestExecution(true, false);
    Parser parser(directory / "main.gibs", false, {}, project);

    ASSERT_EQ(parser.status(), AppError::NoError);
    parser.parse();

    const auto testTarget =
        std::find_if(project->commands.begin(), project->commands.end(),
                     [](const Command &command)
                     {
                         return command.type == Syntax::Command::Executable &&
                                command.targetId.name() == "test_test_suite";
                     });
    ASSERT_NE(testTarget, project->commands.end());
    EXPECT_EQ(testTarget->executable().outputPath.filename(), "test_test_suite");
    EXPECT_TRUE(std::any_of(project->commands.begin(), project->commands.end(),
                            [&directory](const Command &command)
                            {
                                return command.type == Syntax::Command::Source &&
                                       command.object().sourcePath ==
                                           directory / "test suite" / "main.cpp";
                            }));

    processor->waitForFinished();
    std::filesystem::remove_all(directory);
}

#include <algorithm>
#include <chrono>
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

TEST(parsing, ParserResolvesProcessorHeaderProcessIncludePath)
{
    Log::setLogLevel(Log::Type::Silent);

    auto processor = std::make_shared<Processor>();
    processor->setDryRun(true);
    auto project = std::make_shared<Project>(processor);

    const auto projectPath = std::filesystem::path(__FILE__).parent_path() /
                             std::filesystem::path("../../../main.gibs");
    const auto projectRoot =
        std::filesystem::absolute(projectPath).parent_path().lexically_normal();

    Parser parser(projectPath, true, {}, project);
    ASSERT_EQ(parser.status(), AppError::NoError);

    parser.parse();

    const auto source = std::find_if(
        project->commands.begin(), project->commands.end(),
        [](const Command &command)
        {
            return command.type == Syntax::Command::Source &&
                   command.object().sourcePath.filename() == "processor.cpp";
        });
    ASSERT_NE(source, project->commands.end());
    EXPECT_NE(std::find(source->object().includePaths.begin(),
                        source->object().includePaths.end(),
                        (projectRoot / "libraries/process").string()),
              source->object().includePaths.end());
    EXPECT_NE(std::find(source->object().includePaths.begin(),
                        source->object().includePaths.end(),
                        (projectRoot / "libraries/streamer").string()),
              source->object().includePaths.end());
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

    const auto directory =
        std::filesystem::temp_directory_path() / "gibs-invalid-command-test";
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

TEST(parsing, ParserDoesNotRewriteUnchangedGeneratedGTestMain)
{
    Log::setLogLevel(Log::Type::Silent);

    const auto testName = "gtest-cache-" +
                          std::to_string(testing::UnitTest::GetInstance()->random_seed());
    const auto directory = std::filesystem::temp_directory_path() / testName;
    const auto suiteDirectory = directory / testName;
    std::filesystem::create_directories(suiteDirectory);
    {
        std::ofstream projectFile(directory / "main.gibs");
        projectFile << "tests directory " << testName << " gtest\n";
        std::ofstream testSource(suiteDirectory / "test.cpp");
        testSource << "TEST(GeneratedMain, Exists) {}\n";
    }

    auto firstProcessor = std::make_shared<Processor>();
    firstProcessor->setDryRun(true);
    auto firstProject = std::make_shared<Project>(firstProcessor);
    firstProject->setTestExecution(true, false);
    Parser firstParser(directory / "main.gibs", false, {}, firstProject);
    ASSERT_EQ(firstParser.status(), AppError::NoError);
    firstParser.parse();

    const auto generatedMainCommand =
        std::find_if(firstProject->commands.begin(), firstProject->commands.end(),
                     [](const Command &command)
                     {
                         return command.type == Syntax::Command::Source &&
                                command.object().sourcePath.filename() == "gtest_main.cpp";
                     });
    ASSERT_NE(generatedMainCommand, firstProject->commands.end());
    const auto generatedMain = generatedMainCommand->object().sourcePath;
    ASSERT_TRUE(std::filesystem::exists(generatedMain));

    const auto preservedTime =
        std::filesystem::file_time_type::clock::now() - std::chrono::hours(24);
    std::filesystem::last_write_time(generatedMain, preservedTime);

    auto secondProcessor = std::make_shared<Processor>();
    secondProcessor->setDryRun(true);
    auto secondProject = std::make_shared<Project>(secondProcessor);
    secondProject->setTestExecution(true, false);
    Parser secondParser(directory / "main.gibs", false, {}, secondProject);
    ASSERT_EQ(secondParser.status(), AppError::NoError);
    secondParser.parse();

    EXPECT_EQ(std::filesystem::last_write_time(generatedMain), preservedTime);

    std::error_code error;
    std::filesystem::remove_all(generatedMain.parent_path(), error);
    std::filesystem::remove_all(directory, error);
}

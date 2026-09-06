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

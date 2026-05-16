#include <algorithm>
#include <filesystem>
#include <gtest/gtest.h>

#include <logger/log.h>
#include <parsing/parser.h>
#include <processing/processor.h>
#include <project/project.h>

TEST(parsing, ParserInheritsFeatureDefinesToCompileCommands)
{
    Log::setLogLevel(Log::Type::Verbose);

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

    bool compileCommandFound = false;
    for (const auto &command : project->commands)
    {
        if (command.type == Syntax::Command::Source)
        {
            compileCommandFound = true;
            const auto &defines = command.object().defines;
            EXPECT_NE(std::find(defines.begin(), defines.end(), "MY_FEATURE"),
                      defines.end());
        }
    }

    EXPECT_TRUE(compileCommandFound);
}

#include "tools/log.h"
#include <gtest/gtest.h>

#include <tools/commandline.h>

#include <string>
#include <vector>

using StringList = std::vector<std::string>;

TEST(test_commandline, test_toStringList)
{
    constexpr int argc = 5;
    char *argv[] = {(char *)"a", (char *)"b",   (char *)"cde",
                    (char *)"f", (char *)"-gh", NULL};
    const auto result = CommandLine::toStringList(argc, argv);
    const StringList toCompare{"a", "b", "cde", "f", "-gh"};

    EXPECT_EQ(result, toCompare);
}

TEST(test_commandline, test_CommandLine)
{
    const CommandLine cmd({"-q"});

    EXPECT_TRUE(cmd.parsedFlagsText().size() > 0);
    EXPECT_TRUE(cmd.helpText().size() > 0);
    EXPECT_TRUE(cmd.versionText().size() > 0);
    EXPECT_TRUE(cmd.input().empty());
    EXPECT_EQ(cmd.logLevel(), Log::Type::Information);

    EXPECT_TRUE(cmd.isValid());
    EXPECT_FALSE(cmd.hasHelp());
    EXPECT_FALSE(cmd.hasVersion());
    EXPECT_FALSE(cmd.runImmediately());
    EXPECT_FALSE(cmd.isDebug());
    EXPECT_TRUE(cmd.isQuickMode());
    EXPECT_TRUE(cmd.colorfulLogs());
}

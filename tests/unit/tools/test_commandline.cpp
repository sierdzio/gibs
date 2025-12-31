#include <gtest/gtest.h>

#include <logger/log.h>
#include <tools/commandline.h>
#include <tools/stringlist.h>

TEST(test_commandline, test_toStringList)
{
    {
        constexpr int argc = 5;
        char *argv[] = {(char *)"a", (char *)"b",   (char *)"cde",
                        (char *)"f", (char *)"-gh", NULL};
        const auto result = CommandLine::toStringList(argc, argv);
        const StringList toCompare{"a", "b", "cde", "f", "-gh"};

        EXPECT_EQ(result, toCompare);
    }

    {
        // No quotes
        constexpr int argc = 4;
        char *argv[] = {(char *)"a", (char *)"path", (char *)"with",
                        (char *)"spaces/main.cpp", NULL};
        const auto result = CommandLine::toStringList(argc, argv);
        const StringList toCompare{"a", "path", "with", "spaces/main.cpp"};

        EXPECT_EQ(result, toCompare);
    }

    {
        // No closing quote
        constexpr int argc = 4;
        char *argv[] = {(char *)"\"a", (char *)"path", (char *)"with",
                        (char *)"spaces/main.cpp", NULL};
        const auto result = CommandLine::toStringList(argc, argv);
        const StringList toCompare{"a path with spaces/main.cpp"};

        EXPECT_EQ(result, toCompare);
    }

    {
        constexpr int argc = 4;
        char *argv[] = {(char *)"\"a", (char *)"path", (char *)"with",
                        (char *)"spaces/main.cpp\"", NULL};
        const auto result = CommandLine::toStringList(argc, argv);
        const StringList toCompare{"a path with spaces/main.cpp"};

        EXPECT_EQ(result, toCompare);
    }

    // TODO: test a case where there are too many quotes

    // TODO: test cases where file or folder name does contain a quote

    // TODO: test single quotes
}

TEST(test_commandline, test_CommandLine)
{
    {
        const CommandLine cmd({});

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
        EXPECT_FALSE(cmd.isQuickMode());
        EXPECT_TRUE(cmd.colorfulLogs());
    }

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

    {
        const CommandLine cmd({"-h", "-v", "-r", "-d", "-q", "--verbose", "--log-level",
                               "verbose", "--no-color", "main.cpp"});

        EXPECT_TRUE(cmd.parsedFlagsText().size() > 0);
        EXPECT_TRUE(cmd.helpText().size() > 0);
        EXPECT_TRUE(cmd.versionText().size() > 0);
        EXPECT_EQ(cmd.input(), "main.cpp");
        EXPECT_EQ(cmd.logLevel(), Log::Type::Verbose);

        EXPECT_TRUE(cmd.isValid());
        EXPECT_TRUE(cmd.hasHelp());
        EXPECT_TRUE(cmd.hasVersion());
        EXPECT_TRUE(cmd.runImmediately());
        EXPECT_TRUE(cmd.isDebug());
        EXPECT_TRUE(cmd.isQuickMode());
        EXPECT_FALSE(cmd.colorfulLogs());
    }

    {
        const CommandLine cmd({"--verbose", "--log-level", "debug"});

        EXPECT_TRUE(cmd.isValid());
        EXPECT_EQ(cmd.logLevel(), Log::Type::Debug);
    }

    {
        const CommandLine cmd({"--log-level", "debug", "--verbose"});

        EXPECT_TRUE(cmd.isValid());
        EXPECT_EQ(cmd.logLevel(), Log::Type::Verbose);
    }
}

TEST(test_commandline, test_paths)
{
    const std::string exe {"executable"};

    {
        const std::string path{"path/to/main.cpp"};
        const CommandLine cmd({exe, path});

        EXPECT_TRUE(cmd.isValid());
        EXPECT_EQ(cmd.input(), path);
    }

    {
        const std::string path{"\\\\localhost\\some\\weird\\path\\to\\main.cpp"};
        const CommandLine cmd({exe, path});

        EXPECT_TRUE(cmd.isValid());
        EXPECT_EQ(cmd.input(), path);
    }

    {
        const std::string path{"path with spaces/main.cpp"};
        const CommandLine cmd({exe, path});

        EXPECT_TRUE(cmd.isValid());
        EXPECT_EQ(cmd.input(), path);
    }
}

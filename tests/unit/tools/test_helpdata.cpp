#include <gtest/gtest.h>

#include <tools/helpdata.h>
#include <tools/stringlist.h>

TEST(helpdata, emptyFormattedOutput)
{
    HelpData help;
    EXPECT_EQ(help.formatted(80), "");
}

TEST(helpdata, singleEntryFormatting)
{
    HelpData help;
    help.addEntry({"-h", "--help"}, "Show help text.");

    const auto result = help.formatted(80);
    const std::string expected = "  -h, --help  Show help text. \n";

    EXPECT_EQ(result, expected);
}

TEST(helpdata, introAndWrappedDescription)
{
    HelpData help;
    help.addIntro("Intro text that is longer than a narrow width.\n");
    help.addEntry({"-a", "--alpha"}, "This is a long help description that will wrap.");

    const auto result = help.formatted(30);
    const std::string expected =
        "Intro text that is longer \nthan a narrow width.\n  -a, --alpha  This is a long "
        "\n               help \n               description \n               that will "
        "\n               wrap. \n";

    EXPECT_EQ(result, expected);
}

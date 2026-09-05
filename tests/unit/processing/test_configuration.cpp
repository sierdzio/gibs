#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include <processing/configuration.h>

namespace
{
class ConfigurationTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        directory = std::filesystem::temp_directory_path() /
                    ("gibs-configuration-test-" +
                     std::to_string(testing::UnitTest::GetInstance()->random_seed()));
        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(directory);
    }

    std::filesystem::path directory;
};
} //namespace

TEST_F(ConfigurationTest, ReplacesAllOccurrencesAndPreservesText)
{
    const auto input = directory / "template.h.in";
    const auto output = directory / "generated.h";

    std::ofstream(input) << "#define VERSION ${VERSION}\n#define COPY ${VERSION}\n";

    std::string error;
    EXPECT_TRUE(ConfigurationGenerator::generate(input, output, {{"${VERSION}", "1.2.3"}},
                                                 &error))
        << error;

    std::ifstream generated(output);
    EXPECT_EQ(std::string(std::istreambuf_iterator<char>(generated), {}),
              "#define VERSION 1.2.3\n#define COPY 1.2.3\n");
}

TEST_F(ConfigurationTest, FailsWhenTokenIsMissing)
{
    const auto input = directory / "template.h.in";
    const auto output = directory / "generated.h";
    std::ofstream(input) << "unchanged\n";

    std::string error;
    EXPECT_FALSE(ConfigurationGenerator::generate(input, output,
                                                  {{"${MISSING}", "value"}}, &error));
    EXPECT_NE(error.find("${MISSING}"), std::string::npos);
    EXPECT_FALSE(std::filesystem::exists(output));
}

TEST_F(ConfigurationTest, CreatesOutputDirectories)
{
    const auto input = directory / "template.txt";
    const auto output = directory / "nested" / "generated.txt";
    std::ofstream(input) << "${VALUE}";

    EXPECT_TRUE(
        ConfigurationGenerator::generate(input, output, {{"${VALUE}", "configured"}}));
    EXPECT_TRUE(std::filesystem::exists(output));
}

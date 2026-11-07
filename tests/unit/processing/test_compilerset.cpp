#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include <processing/compilerset.h>

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

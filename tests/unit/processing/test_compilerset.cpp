#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>

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

std::filesystem::path defaultCompilerSetDirectory(const std::filesystem::path &home)
{
#ifdef __APPLE__
    return home / "Library" / "Application Support" / "gibs" / "compiler-sets";
#else
    return home / ".config" / "gibs" / "compiler-sets";
#endif
}
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

#if defined(__unix__) || defined(__APPLE__)
TEST_F(CompilerSetIniTest, CompilerSetCreatesDefaultDirectoryAndSeedsDefinitions)
{
    const auto *home = std::getenv("HOME");
    const std::optional<std::string> originalHome =
        home == nullptr ? std::nullopt : std::make_optional(home);
    ASSERT_EQ(::setenv("HOME", directory.c_str(), 1), 0);

    const auto configDirectory = defaultCompilerSetDirectory(directory);
    EXPECT_FALSE(std::filesystem::exists(configDirectory));

    CompilerSet::initialize();

    EXPECT_TRUE(std::filesystem::is_directory(configDirectory));
    EXPECT_TRUE(std::filesystem::is_regular_file(configDirectory / "gcc.ini"));
    EXPECT_TRUE(std::filesystem::is_regular_file(configDirectory / "clang.ini"));
    EXPECT_TRUE(std::filesystem::is_regular_file(configDirectory / "apple-clang.ini"));

    if (originalHome.has_value())
    {
        EXPECT_EQ(::setenv("HOME", originalHome->c_str(), 1), 0);
    }
    else
    {
        EXPECT_EQ(::unsetenv("HOME"), 0);
    }
    CompilerSet::initialize();
}

TEST_F(CompilerSetIniTest, CompilerSetUsesPlatformDefaultFromConfigurationDirectory)
{
    const auto configDirectory = defaultCompilerSetDirectory(directory);
    std::filesystem::create_directories(configDirectory);
#ifdef __APPLE__
    const auto name = "apple-clang";
#else
    const auto name = "gcc";
#endif

    std::ofstream output(configDirectory / (std::string(name) + ".ini"));
    output << "[compiler-set]\n";
    output << "name = " << name << '\n';
    output << "compiler = configured-default-compiler\n";
    output << "archiver = configured-default-archiver\n";
    output << "ranlib = configured-default-ranlib\n";
    output << "execution = Parallel\n";
    output.close();

    const auto *home = std::getenv("HOME");
    const std::optional<std::string> originalHome =
        home == nullptr ? std::nullopt : std::make_optional(home);
    ASSERT_EQ(::setenv("HOME", directory.c_str(), 1), 0);

    CompilerSet::initialize();
    const auto set = CompilerSet::defaultForPlatform();

    EXPECT_EQ(set.name, name);
    EXPECT_EQ(set.compilerCommand, "configured-default-compiler");
    EXPECT_EQ(set.archiverCommand, "configured-default-archiver");
    EXPECT_EQ(set.ranlibCommand, "configured-default-ranlib");
    EXPECT_EQ(set.commandExecution, CompilerSet::CommandExecution::Parallel);

    if (originalHome.has_value())
    {
        EXPECT_EQ(::setenv("HOME", originalHome->c_str(), 1), 0);
    }
    else
    {
        EXPECT_EQ(::unsetenv("HOME"), 0);
    }
    CompilerSet::initialize();
}

TEST_F(CompilerSetIniTest, CompilerSetCachesDefinitionsFromDefaultLocation)
{
    const auto configDirectory = defaultCompilerSetDirectory(directory);
    std::filesystem::create_directories(configDirectory);

    const auto path = configDirectory / "custom-compiler-set.ini";
    std::ofstream output(path);
    output << "[compiler-set]\n";
    output << "name = cached-custom\n";
    output << "compiler = cached-compiler\n";
    output << "archiver = cached-archiver\n";
    output << "ranlib = cached-ranlib\n";
    output << "execution = Parallel\n";
    output.close();

    const auto *home = std::getenv("HOME");
    const std::optional<std::string> originalHome =
        home == nullptr ? std::nullopt : std::make_optional(home);
    ASSERT_EQ(::setenv("HOME", directory.c_str(), 1), 0);

    CompilerSet::initialize();
    EXPECT_TRUE(CompilerSet::isKnownName("CACHED-CUSTOM"));

    const auto set = CompilerSet::fromName("Cached-Custom");
    EXPECT_EQ(set.name, "cached-custom");
    EXPECT_EQ(set.compilerCommand, "cached-compiler");
    EXPECT_EQ(set.archiverCommand, "cached-archiver");
    EXPECT_EQ(set.ranlibCommand, "cached-ranlib");
    EXPECT_EQ(set.commandExecution, CompilerSet::CommandExecution::Parallel);

    if (originalHome.has_value())
    {
        EXPECT_EQ(::setenv("HOME", originalHome->c_str(), 1), 0);
    }
    else
    {
        EXPECT_EQ(::unsetenv("HOME"), 0);
    }
    CompilerSet::initialize();
}
#endif

TEST(processing, CompilerSetSequentialCommandsDefault)
{
    const auto set = CompilerSet::fromName("gcc");
    EXPECT_EQ(set.commandExecution, CompilerSet::CommandExecution::Sequential);
}

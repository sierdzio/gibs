#include <gtest/gtest.h>

#include <processing/buildcache.h>
#include <processing/processor.h>
#include <project/command.h>

#include <chrono>
#include <filesystem>
#include <fstream>

namespace
{
class BuildCacheTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        _directory = std::filesystem::temp_directory_path() /
                     ("gibs-build-cache-" +
                      std::to_string(testing::UnitTest::GetInstance()->random_seed()));
        std::filesystem::create_directories(_directory);
    }

    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove_all(_directory, error);
    }

    std::filesystem::path _directory;
};
} //namespace

TEST_F(BuildCacheTest, PersistsSourceAndTargetTimestamps)
{
    const auto source = _directory / "main.cpp";
    const auto object = _directory / "main.o";
    const auto target = _directory / "app";

    std::ofstream(source) << "int main() {}\n";
    std::ofstream(object) << "object";
    std::ofstream(target) << "target";

    BuildCache cache;
    cache.setDirectory(_directory);
    cache.markSourceCompiled(source, object);
    cache.markTargetLinked(target);
    EXPECT_TRUE(cache.save());

    BuildCache restored;
    restored.setDirectory(_directory);
    EXPECT_TRUE(restored.isSourceUpToDate(source, object));
    EXPECT_TRUE(restored.isTargetUpToDate(target));
}

TEST_F(BuildCacheTest, DetectsModifiedSource)
{
    const auto source = _directory / "main.cpp";
    const auto object = _directory / "main.o";

    std::ofstream(source) << "int main() {}\n";
    std::ofstream(object) << "object";

    BuildCache cache;
    cache.setDirectory(_directory);
    cache.markSourceCompiled(source, object);
    EXPECT_TRUE(cache.save());

    const auto oldTime = std::filesystem::last_write_time(source);
    std::ofstream(source, std::ios::app) << "// changed\n";
    std::filesystem::last_write_time(source, oldTime + std::chrono::seconds(2));

    BuildCache restored;
    restored.setDirectory(_directory);
    EXPECT_FALSE(restored.isSourceUpToDate(source, object));
}

TEST_F(BuildCacheTest, RequiresTargetOutputToExist)
{
    BuildCache cache;
    cache.setDirectory(_directory);

    const auto target = _directory / "missing";
    EXPECT_FALSE(cache.isTargetUpToDate(target));
}

TEST_F(BuildCacheTest, ProcessorReportsCachedAndUncachedCommands)
{
    const auto source = _directory / "main.cpp";
    const auto object = _directory / "main.o";
    std::ofstream(source) << "int main() {}\n";
    std::ofstream(object) << "object";

    BuildCache cache;
    cache.setDirectory(_directory);
    cache.markSourceCompiled(source, object);
    EXPECT_TRUE(cache.save());

    Processor processor;
    processor.setDryRun(true);
    processor.setBuildDirectory(_directory);

    Command cached;
    cached.append("source");
    cached.append("main.cpp");
    Paths paths;
    paths.workingDirectory = _directory;
    paths.buildDirectory = _directory;
    cached.finalize({}, paths);
    processor.schedule(cached);
    processor.waitForFinished();

    EXPECT_EQ(processor.cachedCommandCount(), 1u);
    EXPECT_EQ(processor.uncachedCommandCount(), 0u);
}

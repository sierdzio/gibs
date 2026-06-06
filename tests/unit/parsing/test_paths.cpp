#include <gtest/gtest.h>

#include <parsing/paths.h>

#include <filesystem>
#include <fstream>

TEST(paths, absolutePath_behaviour)
{
    namespace fs = std::filesystem;

    const auto tmp = fs::temp_directory_path() / "gibs_test_paths_tmp";
    fs::create_directories(tmp);

    const auto proj = tmp / "proj";
    const auto wd = tmp / "wd";
    fs::create_directories(proj);
    fs::create_directories(wd);

    const auto projFile = proj / "projfile.h";
    {
        std::ofstream out(projFile);
        out << "x";
    }

    const auto incDir = wd / "inc";
    fs::create_directories(incDir);
    const auto incFile = incDir / "inc.h";
    {
        std::ofstream out(incFile);
        out << "x";
    }

    Paths p;
    p.workingDirectory = wd;
    p.projectDirectory = proj;
    p.includePaths.emplace_back("inc");

    // Absolute path is returned as-is
    EXPECT_EQ(p.absolutePath(fs::path("/tmp/somefile")).string(), "/tmp/somefile");

    // Found in project directory
    EXPECT_EQ(p.absolutePath("projfile.h"), (proj / "projfile.h").lexically_normal());

    // Found in include path under working directory
    EXPECT_EQ(p.absolutePath("inc.h"), (wd / "inc" / "inc.h").lexically_normal());

    // Not found -> fallback to working directory
    EXPECT_EQ(p.absolutePath("nofile.h"), (wd / "nofile.h").lexically_normal());

    fs::remove_all(tmp);
}

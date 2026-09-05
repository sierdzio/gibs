#pragma once

#include <filesystem>
#include <ostream>
#include <vector>

struct Paths
{
    std::filesystem::path workingDirectory = std::filesystem::current_path();
    std::filesystem::path projectDirectory;
    std::filesystem::path projectFile;
    std::filesystem::path projectEntryPoint;
    std::filesystem::path buildDirectory = std::filesystem::current_path() / "build";
    // TODO: should be per target (library, executable) or even more granular to speed
    // things up?
    std::vector<std::filesystem::path> includePaths;

    std::filesystem::path absolutePath(const std::filesystem::path &inputPath) const;
};

std::ostream &operator<<(std::ostream &stream,
                         const std::vector<std::filesystem::path> &paths);
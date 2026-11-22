#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

class BuildCache
{
  public:
    void setDirectory(const std::filesystem::path &buildDirectory);

    bool isSourceUpToDate(const std::filesystem::path &source,
                          const std::filesystem::path &output) const;
    bool isTargetUpToDate(const std::filesystem::path &target) const;

    void markSourceCompiled(const std::filesystem::path &source,
                            const std::filesystem::path &output);
    void markTargetLinked(const std::filesystem::path &target);
    bool save();

  private:
    static std::string timestamp(const std::filesystem::path &path);
    static std::filesystem::path normalized(const std::filesystem::path &path);

    std::filesystem::path _directory;
    std::unordered_map<std::string, std::string> _timestamps;
    bool _loaded = false;
    bool _dirty = false;

    void load();
};

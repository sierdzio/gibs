#include "buildcache.h"

#include <streamer/ini.h>

#include <logger/log.h>

#include <chrono>
#include <system_error>

void BuildCache::setDirectory(const std::filesystem::path &buildDirectory)
{
    _directory = buildDirectory.lexically_normal();
    _loaded = false;
    _timestamps.clear();
}

std::filesystem::path BuildCache::filePath() const
{
    return (_directory / ".gibs.cache").lexically_normal();
}

std::filesystem::path BuildCache::normalized(const std::filesystem::path &path)
{
    std::error_code error;
    auto result = path.is_absolute() ? path : std::filesystem::absolute(path, error);
    return result.lexically_normal();
}

std::string BuildCache::timestamp(const std::filesystem::path &path)
{
    std::error_code error;
    if (not std::filesystem::is_regular_file(path, error))
    {
        return {};
    }

    const auto modified = std::filesystem::last_write_time(path, error);
    if (error)
    {
        return {};
    }

    return std::to_string(modified.time_since_epoch().count());
}

void BuildCache::load()
{
    if (_loaded)
    {
        return;
    }
    _loaded = true;

    if (_directory.empty())
    {
        _directory = std::filesystem::current_path() / "build";
    }

    const auto cachePath = filePath();
    if (not std::filesystem::is_regular_file(cachePath))
    {
        return;
    }

    Ini<Object> cache;
    if (not cache.read(cachePath))
    {
        return;
    }

    const auto section = cache.find("files");
    if (section == cache.end())
    {
        return;
    }

    try
    {
        for (const auto &[path, value] : section->second.asObject())
        {
            if (value.type() == typeid(std::string))
            {
                _timestamps.emplace(path, value.toString());
            }
        }
    }
    catch (const std::bad_variant_access &)
    {
        Log::warning("Ignoring malformed build cache:", cachePath.string());
    }
}

bool BuildCache::isSourceUpToDate(const std::filesystem::path &source,
                                  const std::filesystem::path &output) const
{
    const_cast<BuildCache *>(this)->load();

    const auto sourcePath = normalized(source);
    const auto outputPath = normalized(output);
    const auto sourceTimestamp = timestamp(sourcePath);
    if (sourceTimestamp.empty() || timestamp(outputPath).empty())
    {
        return false;
    }

    const auto cached = _timestamps.find(sourcePath.string());
    return cached != _timestamps.end() && cached->second == sourceTimestamp &&
           timestamp(outputPath).empty() == false;
}

bool BuildCache::isTargetUpToDate(const std::filesystem::path &target) const
{
    const_cast<BuildCache *>(this)->load();

    const auto targetPath = normalized(target);
    const auto targetTimestamp = timestamp(targetPath);
    if (targetTimestamp.empty())
    {
        return false;
    }

    const auto cached = _timestamps.find(targetPath.string());
    if (cached == _timestamps.end() || cached->second != targetTimestamp)
    {
        return false;
    }

    return true;
}

void BuildCache::markSourceCompiled(const std::filesystem::path &source,
                                    const std::filesystem::path &output)
{
    load();
    const auto sourcePath = normalized(source);
    const auto sourceTimestamp = timestamp(sourcePath);
    if (not sourceTimestamp.empty())
    {
        _timestamps[sourcePath.string()] = sourceTimestamp;
        _dirty = true;
    }

    const auto outputPath = normalized(output);
    const auto outputTimestamp = timestamp(outputPath);
    if (not outputTimestamp.empty())
    {
        _timestamps[outputPath.string()] = outputTimestamp;
        _dirty = true;
    }
}

void BuildCache::markTargetLinked(const std::filesystem::path &target)
{
    load();
    const auto targetPath = normalized(target);
    const auto targetTimestamp = timestamp(targetPath);
    if (not targetTimestamp.empty())
    {
        _timestamps[targetPath.string()] = targetTimestamp;
        _dirty = true;
    }
}

bool BuildCache::save()
{
    if (not _dirty)
    {
        return true;
    }

    if (_directory.empty())
    {
        _directory = std::filesystem::current_path() / "build";
    }

    std::error_code error;
    std::filesystem::create_directories(_directory, error);
    if (error)
    {
        Log::error("Could not create build directory:", _directory.string());
        return false;
    }

    Object files;
    for (const auto &[path, modified] : _timestamps)
    {
        files.insert(path, Value{modified});
    }

    Ini<Object> cache;
    cache.insert("files", Value{files});
    const auto saved = cache.write(filePath());
    if (saved)
    {
        _dirty = false;
    }
    else
    {
        Log::error("Failed to write build cache:",
                   (_directory / ".gibs.cache").lexically_normal().string());
    }
    return saved;
}

#include "compilerset.h"
#include "tools/standardpaths.h"
#include "tools/tools.h"

#include <logger/log.h>

//i include libraries/streamer
#include <streamer/ini.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::string_view SectionName = "compiler-set";
constexpr std::string_view Name = "name";
constexpr std::string_view Compiler = "compiler";
constexpr std::string_view Archiver = "archiver";
constexpr std::string_view Ranlib = "ranlib";
constexpr std::string_view Execution = "execution";

struct CompilerSetCache
{
    std::mutex mutex;
    std::mutex initializationMutex;
    std::map<std::string, CompilerSet> sets;
    bool initialized = false;
};

CompilerSetCache &compilerSetCache()
{
    static CompilerSetCache cache;
    return cache;
}

std::optional<CompilerSet> readCompilerSet(const std::filesystem::path &path)
{
    Ini<Object> ini;
    if (not ini.read(path))
    {
        return std::nullopt;
    }

    for (const auto &[section, entry] : ini)
    {
        if (section != SectionName)
        {
            continue;
        }

        const auto &object = entry.asObject();
        if (not object.contains(Name.data()) or not object.contains(Compiler.data()) or
            not object.contains(Archiver.data()) or not object.contains(Ranlib.data()) or
            not object.contains(Execution.data()))
        {
            Log::error("Compiler set INI file is missing required entries:",
                       path.string());
            return std::nullopt;
        }

        CompilerSet set;
        set.name = object.at(Name.data()).toString();
        set.compilerCommand = object.at(Compiler.data()).toString();
        set.archiverCommand = object.at(Archiver.data()).toString();
        set.ranlibCommand = object.at(Ranlib.data()).toString();
        const auto &execution = object.at(Execution.data()).toString();
        set.commandExecution = execution == "Parallel"
                                   ? CompilerSet::CommandExecution::Parallel
                                   : CompilerSet::CommandExecution::Sequential;
        return set;
    }

    return std::nullopt;
}

std::optional<std::filesystem::path> expandHomeDirectory(std::string_view path)
{
    if (not path.starts_with("~/"))
    {
        return std::filesystem::path(path);
    }

#if defined(_WIN32)
    char *homeValue = nullptr;
    std::size_t homeValueSize = 0;
    if (_dupenv_s(&homeValue, &homeValueSize, "USERPROFILE") != 0 or
        homeValue == nullptr)
    {
        return std::nullopt;
    }
    const std::unique_ptr<char, decltype(&std::free)> home(homeValue, &std::free);
#else
    const auto *home = std::getenv("HOME");
    if (home == nullptr)
    {
        return std::nullopt;
    }
#endif

#if defined(_WIN32)
    return std::filesystem::path(home.get()) / path.substr(2);
#else
    return std::filesystem::path(home) / path.substr(2);
#endif
}

std::string_view platformDefaultName()
{
#ifdef __APPLE__
    return "apple-clang";
#else
    return "gcc";
#endif
}

std::vector<CompilerSet> builtInCompilerSets()
{
    return {
        {"gcc", "g++", "ar", "ranlib", CompilerSet::CommandExecution::Sequential},
        {"clang", "clang++", "ar", "ranlib", CompilerSet::CommandExecution::Sequential},
        {"apple-clang", "clang++", "ar", "ranlib",
         CompilerSet::CommandExecution::Sequential},
    };
}

void writeDefaultCompilerSet(const std::filesystem::path &directory,
                             const CompilerSet &set)
{
    const auto path = directory / (set.name + ".ini");
    std::error_code error;
    if (std::filesystem::exists(path, error))
    {
        return;
    }
    if (error)
    {
        Log::warning("Could not check default compiler set file:", path.string(),
                     error.message());
        return;
    }

    std::ofstream output(path);
    if (not output)
    {
        Log::error("Could not create default compiler set file:", path.string());
        return;
    }

    output << "[compiler-set]\n"
           << "name = " << set.name << '\n'
           << "compiler = " << set.compilerCommand << '\n'
           << "archiver = " << set.archiverCommand << '\n'
           << "ranlib = " << set.ranlibCommand << '\n'
           << "execution = "
           << (set.commandExecution == CompilerSet::CommandExecution::Parallel
                   ? "Parallel"
                   : "Sequential")
           << '\n';
    if (not output)
    {
        Log::error("Could not write default compiler set file:", path.string());
    }
}

void ensureCompilerSetsInitialized()
{
    auto &cache = compilerSetCache();
    {
        const std::lock_guard lock(cache.mutex);
        if (cache.initialized)
        {
            return;
        }
    }

    CompilerSet::initialize();
}
} //namespace

void CompilerSet::initialize()
{
    auto &cache = compilerSetCache();
    const std::lock_guard initializationLock(cache.initializationMutex);
    std::map<std::string, CompilerSet> sets;

    for (const auto searchPath : StandardPaths::compilerSetsPaths())
    {
        const auto expandedDirectory = expandHomeDirectory(searchPath);
        if (not expandedDirectory.has_value())
        {
            Log::warning("Could not resolve compiler set directory:", searchPath);
            continue;
        }
        const auto &directory = *expandedDirectory;

        std::error_code error;
        std::filesystem::create_directories(directory, error);
        if (error)
        {
            Log::error("Could not create compiler set directory:", directory.string(),
                       error.message());
            continue;
        }

        for (const auto &set : builtInCompilerSets())
        {
            writeDefaultCompilerSet(directory, set);
        }

        std::vector<std::filesystem::path> paths;
        for (std::filesystem::directory_iterator entry(directory, error), end;
             not error and entry != end; entry.increment(error))
        {
            if (entry->is_regular_file(error) and entry->path().extension() == ".ini")
            {
                paths.push_back(entry->path());
            }
            if (error)
            {
                Log::warning("Could not inspect compiler set file in:", directory.string(),
                             error.message());
                error.clear();
            }
        }
        if (error)
        {
            Log::warning("Could not read compiler set directory:", directory.string(),
                         error.message());
        }

        std::sort(paths.begin(), paths.end());
        for (const auto &path : paths)
        {
            const auto set = readCompilerSet(path);
            if (not set.has_value() or set->name.empty())
            {
                continue;
            }

            const auto key = Tools::toLower(set->name);
            if (sets.contains(key))
            {
                Log::warning("Ignoring duplicate compiler set name:", set->name,
                             "from", path.string());
                continue;
            }
            sets.emplace(key, *set);
        }
    }

    for (const auto &set : builtInCompilerSets())
    {
        sets.try_emplace(Tools::toLower(set.name), set);
    }

    const std::lock_guard lock(cache.mutex);
    cache.sets = std::move(sets);
    cache.initialized = true;
}

CompilerSet CompilerSet::defaultForPlatform()
{
    ensureCompilerSetsInitialized();

    auto &cache = compilerSetCache();
    const std::lock_guard lock(cache.mutex);
    const auto set = cache.sets.find(std::string(platformDefaultName()));
    if (set != cache.sets.end())
    {
        return set->second;
    }

    for (const auto &builtIn : builtInCompilerSets())
    {
        if (builtIn.name == platformDefaultName())
        {
            return builtIn;
        }
    }
    throw std::logic_error("Platform default compiler set is not defined");
}

CompilerSet CompilerSet::fromName(const std::string &name)
{
    ensureCompilerSetsInitialized();
    const auto normalized = Tools::toLower(name);

    auto &cache = compilerSetCache();
    {
        const std::lock_guard lock(cache.mutex);
        const auto cached = cache.sets.find(normalized);
        if (cached != cache.sets.end())
        {
            return cached->second;
        }
    }

    if (std::filesystem::exists(name))
    {
        if (const auto set = readCompilerSet(name))
        {
            return *set;
        }
    }

    return defaultForPlatform();
}

bool CompilerSet::isKnownName(const std::string &name)
{
    ensureCompilerSetsInitialized();
    const auto normalized = Tools::toLower(name);
    auto &cache = compilerSetCache();
    const std::lock_guard lock(cache.mutex);
    return cache.sets.contains(normalized);
}

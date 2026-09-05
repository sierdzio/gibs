#pragma once

#include "parsing/syntax.h"
#include "tools/stringlist.h"

#include <filesystem>
#include <vector>

struct Component
{
    virtual bool isValid(const Syntax::Command type) const = 0;
};

struct ExecutableComponent : public Component
{
    bool isValid(const Syntax::Command type) const override;

    std::string name;
    std::string version;
    std::filesystem::path outputPath;
    StringList objects;
    StringList libraries;
};

struct LibraryComponent : public ExecutableComponent
{
    bool isValid(const Syntax::Command type) const final;

    Syntax::LibraryType type = Syntax::LibraryType::Dynamic;
};

struct ObjectComponent : public Component
{
    bool isValid(const Syntax::Command type) const override;

    std::string source;
    std::filesystem::path sourcePath;
    std::string name;
    StringList includePaths;
    StringList defines;
};

struct IncludeComponent : public Component
{
    bool isValid(const Syntax::Command type) const override;

    // TODO: these checks and results should be cached!
    std::string dirPath() const;
    std::string libraryName() const;

    std::string path; // TODO: or maybe std::filesystem::path?
    // TODO: add -L and -l flags support
    bool isLibrary = false;
};

struct OptionComponent : public Component
{
    bool isValid(const Syntax::Command type) const override;
    std::string define() const;

    std::string name;
    bool defaultValue = false;
    bool isOn = false;
};

struct ConfigurationReplacement
{
    std::string token;
    std::string value;
};

struct ConfigurationComponent final : public Component
{
    bool isValid(const Syntax::Command type) const override;

    std::string input;
    std::string output;
    std::vector<ConfigurationReplacement> replacements;
};

struct ReplacementComponent final : public Component
{
    bool isValid(const Syntax::Command type) const override;

    std::string token;
    std::string value;
};

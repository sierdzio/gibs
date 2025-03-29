#pragma once

#include "parsing/syntax.h"

#include <string>
#include <vector>

struct Component
{
    virtual bool isValid(const Syntax::Command type) const = 0;
};

struct ExecutableComponent : public Component
{
    bool isValid(const Syntax::Command type) const override;

    std::string name;
    std::vector<std::string> objects;
};

struct LibraryComponent : public ExecutableComponent
{
    bool isValid(const Syntax::Command type) const final;

    Syntax::LibraryType type = Syntax::LibraryType::Dynamic;
};

struct ObjectComponent : public Component
{
    bool isValid(const Syntax::Command type) const override;

    std::string name;
};

struct IncludeComponent : public Component
{
    bool isValid(const Syntax::Command type) const override;

    std::string path; // TODO: or maybe std::filesystem::path?
    // TODO: add -L and -l flags support
    bool isLibrary = false;
};

struct OptionComponent : public Component
{
    bool isValid(const Syntax::Command type) const override;

    std::string name;
    bool defaultValue = false;
    bool isOn = false;
};

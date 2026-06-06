#pragma once

#include <algorithm>
#include <filesystem>
#include <ostream>
#include <string>

#define TYPES                                                                            \
    X(Unknown, "unknown")                                                                \
    X(Executable, "executable")                                                          \
    X(Library, "library")

struct TargetId
{
#define X(key, name) key,
    enum class Type
    {
        TYPES
    };
#undef X

    static const std::string typeString(const Type type);
    static Type typeValue(const std::string &string);
    static size_t typesCount();

    TargetId();
    TargetId(std::string &&name, const Type type);

    // TODO: Add root directory for targets, option to set them exclusive or not.
    // Then in Parser take settings into consideration when resolving command inheritance.
    // Note: not sure if this is the right place for this, performance might be hit
    std::filesystem::path rootDirectory() const;
    void setRootDirectory(const std::filesystem::path &path);

    bool operator<=>(const TargetId &other) const = default;
    bool isNull() const;

    const std::string &name() const;
    Type type() const;

  private:
    void setName(const std::string &name);

    std::string _name;
    std::filesystem::path _rootDirectory;
    Type _type = Type::Unknown;
    unsigned int _id = 0;
};

std::ostream &operator<<(std::ostream &stream, const TargetId &id);

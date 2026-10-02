#pragma once

#include "data.h"

#include <filesystem>
#include <ostream>
#include <string>
#include <type_traits>

/*!
Super simple JSON library for C++.

Does not support much, but it is only intended to be used inside of gibs.
*/
template <typename Type>
    requires(std::is_base_of_v<Object, Type> || std::is_base_of_v<Array, Type>)
class Json : public Type
{
  public:
    static std::string serialize(const Object &object);
    static void serialize(const Object &object, std::ostream &output);
    static std::string serialize(const Array &array);
    static void serialize(const Array &array, std::ostream &output);
    static void writeValue(std::ostream &output, const Value &value,
                           bool addTrailingNewline = true);

    bool write(const std::filesystem::path &path) const;
};

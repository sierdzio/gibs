#pragma once

#include <string>
#include <vector>
#include <iostream>

std::ostream &operator<<(std::ostream &stream, const std::vector<std::string> &stringList);

namespace Log
{
    enum class Type {
        Verbose,
        Debug,
        Information,
        Warning,
        Error
    };

    std::string type(const Type type);

    template<typename... Types>
    void verbose(Types&&... args)
    {
        log(Type::Verbose, args...);
    }

    template<typename... Types>
    void debug(Types&&... args)
    {
        log(Type::Debug, args...);
    }

    template<typename... Types>
    void information(Types&&... args)
    {
        log(Type::Information, args...);
    }

    template<typename... Types>
    void warning(Types&&... args)
    {
        log(Type::Warning, args...);
    }

    template<typename... Types>
    void error(Types&&... args)
    {
        log(Type::Error, args...);
    }

    template<typename... Types>
    void log(const Type type, Types&&... args)
    {
        constexpr auto Space = ' ';

        std::cout << Log::type(type) << Space;

        // This is a "loop" lambda
        ([&]{
            std::cout << args << Space;
        } (), ...);

        std::cout << std::endl;
    }
};

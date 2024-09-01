#pragma once

#include <string>

class Parser {
public:
    Parser(std::string &&input);

private:
    std::string _input;
};
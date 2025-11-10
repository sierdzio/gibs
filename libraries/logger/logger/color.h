#pragma once

#include <cstdint>
#include <string>

namespace Log
{
namespace Standard
{
enum class Foreground : uint8_t
{
    Default = 0,
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    White = 37,
    BrightBlack = 90,
    BrightRed = 91,
    BrightGreen = 92,
    BrightYellow = 93,
    BrightBlue = 94,
    BrightMagenta = 95,
    BrightCyan = 96,
    BrightWhite = 97
};

enum class Background : uint8_t
{
    Default = 0,
    Black = 40,
    Red = 41,
    Green = 42,
    Yellow = 43,
    Blue = 44,
    Magenta = 45,
    Cyan = 46,
    White = 47,
    BrightBlack = 100,
    BrightRed = 101,
    BrightGreen = 102,
    BrightYellow = 103,
    BrightBlue = 104,
    BrightMagenta = 105,
    BrightCyan = 106,
    BrightWhite = 107
};
} //namespace Standard

struct Rgb
{
    Rgb() = default;
    Rgb(const uint8_t red, const uint8_t green, const uint8_t blue);

    bool isDefault() const;

    const uint8_t red = 0;
    const uint8_t green = 0;
    const uint8_t blue = 0;

  private:
    const bool _isDefault = true;
};

class Color
{
  public:
    Color() = default;

    Color(const Standard::Foreground foreground,
          const Standard::Background background = Standard::Background::Default);
    Color(const Rgb &foreground, const Rgb &background = {});

    const std::string &ansiEscapeCode() const;
    bool isDefault() const;

  private:
    std::string _ansiEscapeCode;
};
} //namespace Log

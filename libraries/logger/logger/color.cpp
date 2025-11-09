#include "color.h"
#include <string>

namespace
{
constexpr std::string begin = "\033[";
constexpr std::string rgbForeground = "38;2;";
constexpr std::string rgbBackground = "48;2;";
constexpr auto SemiColon = ';';
constexpr auto M = 'm';
} //namespace

namespace Log
{
Rgb::Rgb(const uint8_t red, const uint8_t green, const uint8_t blue)
    : red(red), green(green), blue(blue), _isDefault(false)
{
}

bool Rgb::isDefault() const
{
    return _isDefault;
}

Color::Color(const Standard::Foreground foreground, const Standard::Background background)
{
    const bool bgIsDefault = background == Standard::Background::Default;

    if (foreground == Standard::Foreground::Default and bgIsDefault)
    {
        return;
    }

    if (bgIsDefault)
    {
        _ansiEscapeCode = begin + std::to_string(static_cast<uint8_t>(foreground)) + M;
    }
    else
    {
        _ansiEscapeCode = begin + std::to_string(static_cast<uint8_t>(foreground)) +
                          SemiColon + std::to_string(static_cast<uint8_t>(background)) +
                          M;
    }
}

Color::Color(const Rgb &foreground, const Rgb &background)
{
    /*
        ESC[38;2;⟨r⟩;⟨g⟩;⟨b⟩m Select RGB foreground color
        ESC[48;2;⟨r⟩;⟨g⟩;⟨b⟩m Select RGB background color
    */

    if (not foreground.isDefault())
    {
        _ansiEscapeCode.append(begin + rgbForeground + std::to_string(foreground.red) +
                               SemiColon + std::to_string(foreground.green) + SemiColon +
                               std::to_string(foreground.blue) + M);
    }

    if (not background.isDefault())
    {
        _ansiEscapeCode.append(begin + rgbBackground + std::to_string(background.red) +
                               SemiColon + std::to_string(background.green) + SemiColon +
                               std::to_string(background.blue) + M);
    }
}

const std::string &Color::ansiEscapeCode() const
{
    return _ansiEscapeCode;
}

} //namespace Log
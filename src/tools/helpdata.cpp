#include "helpdata.h"

#include <ranges>
#include <string_view>

namespace
{
const std::string_view Space = " ";
constexpr std::string_view DoubleSpace = "  ";
constexpr std::string_view FlagSeparator = ", ";
}; //namespace

void HelpData::addIntro(const std::string_view &introText)
{
    intro = introText;
}

void HelpData::addEntry(const StringViewList &flags, const std::string_view &explanation)
{
    entries.push_back({flags, explanation});
}

std::string HelpData::formatted(const uint width) const
{
    const auto indent = calculateDescriptionIndent();

    std::string result;

    if (not intro.empty())
    {
        uint emptyIndent = 0;
        appendWordsWithWrapping(result, intro, emptyIndent, width);
    }

    for (const auto &entry : entries)
    {
        appendText(result, entry, indent, width);
    }

    return result;
}

uint HelpData::calculateDescriptionIndent() const
{
    uint result = 0;

    for (const auto &entry : entries)
    {
        uint flagsLength = 0;

        for (const auto &flag : entry.flags)
        {
            flagsLength +=
                static_cast<uint>(flag.size()) + static_cast<uint>(FlagSeparator.size());
        }

        result = std::max(result, flagsLength);
    }

    return result + static_cast<uint>(DoubleSpace.size());
}

void HelpData::appendText(std::string &string, const HelpEntry &entry, const uint indent,
                          const uint width) const
{
    uint currentColumn = 0;

    if (not entry.flags.empty())
    {
        string.append(DoubleSpace);
        currentColumn = static_cast<uint>(DoubleSpace.size());
    }

    bool isFirst = true;
    for (const auto &flag : entry.flags)
    {
        if (isFirst)
        {
            isFirst = false;
        }
        else
        {
            string.append(FlagSeparator);
            currentColumn += static_cast<uint>(FlagSeparator.size());
        }

        string.append(flag);
        currentColumn += static_cast<uint>(flag.size());
    }

    if (not entry.flags.empty())
    {
        string.append(std::string(indent - currentColumn, Space[0]));
        currentColumn = indent;
    }

    appendWordsWithWrapping(string, entry.explanation, currentColumn, width);

    string.push_back('\n');
}

void HelpData::appendWordsWithWrapping(std::string &string,
                                       const std::string_view &toAppend,
                                       uint &currentColumn, const uint maxWidth) const
{
    const auto indent = currentColumn;

    auto words = toAppend | std::views::split(std::string_view{Space}) |
                 std::views::transform([](auto &&str) { return std::string_view(str); });

    for (auto &&word : words)
    {
        currentColumn += static_cast<uint>(word.size());

        if (currentColumn > maxWidth)
        {
            string.push_back('\n');
            string.append(std::string(indent, Space[0]));
            currentColumn = indent;
        }

        string.append(word);
        string.append(Space);
        currentColumn += static_cast<uint>(word.size()) + 1;
    }
}
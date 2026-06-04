#include "helpdata.h"
#include "logger/log.h"

#include <limits>
#include <ranges>
#include <string_view>

namespace
{
const std::string_view Space = " ";
const std::string_view Nl = "\n";
constexpr std::string_view DoubleSpace = "  ";
constexpr std::string_view FlagSeparator = ", ";

unsigned int toUint(const unsigned long long value)
{
    if (value > static_cast<unsigned long long>(std::numeric_limits<unsigned int>::max()))
    {
        throw std::overflow_error("Value is too large to fit in unsigned int");
    }
    return static_cast<unsigned int>(value);
}
}; //namespace

void HelpData::addIntro(const std::string_view &introText)
{
    intro = introText;
}

void HelpData::addEntry(const StringViewList &flags, const std::string_view &explanation)
{
    entries.push_back({flags, explanation});
}

std::string HelpData::formatted(const unsigned int width) const
{
    //Log::error("Requested width:", width);

    const auto indent = calculateDescriptionIndent();

    std::string result;

    if (not intro.empty())
    {
        unsigned int emptyIndent = 0;
        appendWordsWithWrapping(result, intro, emptyIndent, width);
    }

    for (size_t i = 0; i < entries.size(); ++i)
    {
        appendText(result, entries.at(i), indent, width);
        if (i < entries.size() - 1)
        {
            result.push_back('\n');
        }
    }

    return result;
}

unsigned int HelpData::calculateDescriptionIndent() const
{
    unsigned int result = 0;

    for (const auto &entry : entries)
    {
        unsigned int flagsLength = 0;

        for (const auto &flag : entry.flags)
        {
            flagsLength += toUint(flag.size()) + toUint(FlagSeparator.size());
        }

        result = std::max(result, flagsLength);
    }

    return result + toUint(DoubleSpace.size());
}

void HelpData::appendText(std::string &string, const HelpEntry &entry,
                          const unsigned int indent, const unsigned int width) const
{
    unsigned int currentColumn = 0;

    if (not entry.flags.empty())
    {
        string.append(DoubleSpace);
        currentColumn = toUint(DoubleSpace.size());
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
            currentColumn += toUint(FlagSeparator.size());
        }

        string.append(flag);
        currentColumn += toUint(flag.size());
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
                                       unsigned int &currentColumn,
                                       const unsigned int maxWidth) const
{
    const auto indent = currentColumn;

    auto words = toAppend | std::views::split(Space) |
                 std::views::transform([](auto &&str) { return std::string_view(str); });

    for (auto &&word : words)
    {
        // +1 for space after the word
        const auto currentSize = toUint(word.size() + 1);

        if (currentColumn + currentSize > maxWidth)
        {
            string.append(Nl);
            string.append(std::string(indent, Space[0]));
            currentColumn = indent;
        }

        string.append(word);

        if (not word.ends_with(Nl))
        {
            string.append(Space);
            currentColumn += currentSize;
        }
    }
}
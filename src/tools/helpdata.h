#pragma once

#include "stringlist.h"

struct HelpEntry
{
    StringViewList flags;
    std::string_view explanation;
};

class HelpData
{
  public:
    void addIntro(const std::string_view &introText);
    void addEntry(const StringViewList &flags, const std::string_view &explanation);
    std::string formatted(const uint width) const;

  private:
    uint calculateDescriptionIndent() const;
    void appendText(std::string &string, const HelpEntry &entry, const uint indent,
                    const uint width) const;
    void appendWordsWithWrapping(std::string &string, const std::string_view &toAppend,
                                 uint &currentColumn, const uint maxWidth) const;

    std::string_view intro;
    std::vector<HelpEntry> entries;
};
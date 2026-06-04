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
    std::string formatted(const unsigned int width) const;

  private:
    unsigned int calculateDescriptionIndent() const;
    void appendText(std::string &string, const HelpEntry &entry,
                    const unsigned int indent, const unsigned int width) const;
    void appendWordsWithWrapping(std::string &string, const std::string_view &toAppend,
                                 unsigned int &currentColumn,
                                 const unsigned int maxWidth) const;

    std::string_view intro;
    std::vector<HelpEntry> entries;
};
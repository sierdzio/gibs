#include "exceptions/commandexception.h"
#include <gtest/gtest.h>

#include <parsing/syntax.h>

// TODO: MetaEnum? EnumClass? Come up with some clever thingy
TEST(test_syntax, test_commandString)
{
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Invalid), "");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Source), "source");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Library), "library");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Define), "define");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Include), "include");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Executable), "executable");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Feature), "feature");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Option), "option");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Subproject), "subproject");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Tool), "tool");
    EXPECT_EQ(Syntax::commandString(Syntax::Command::Qt), "qt");
    EXPECT_THROW(Syntax::commandString(static_cast<Syntax::Command>(123)),
                 CommandException);
}

TEST(test_syntax, test_commandValue)
{
    EXPECT_EQ(Syntax::commandValue(""), Syntax::Command::Invalid);
    EXPECT_EQ(Syntax::commandValue("source"), Syntax::Command::Source);
    EXPECT_EQ(Syntax::commandValue("library"), Syntax::Command::Library);
    EXPECT_EQ(Syntax::commandValue("define"), Syntax::Command::Define);
    EXPECT_EQ(Syntax::commandValue("include"), Syntax::Command::Include);
    EXPECT_EQ(Syntax::commandValue("executable"), Syntax::Command::Executable);
    EXPECT_EQ(Syntax::commandValue("feature"), Syntax::Command::Feature);
    EXPECT_EQ(Syntax::commandValue("option"), Syntax::Command::Option);
    EXPECT_EQ(Syntax::commandValue("subproject"), Syntax::Command::Subproject);
    EXPECT_EQ(Syntax::commandValue("tool"), Syntax::Command::Tool);
    EXPECT_EQ(Syntax::commandValue("qt"), Syntax::Command::Qt);

    EXPECT_THROW(Syntax::commandValue("abc"), CommandStringException);
    EXPECT_THROW(Syntax::commandValue("Qt"), CommandStringException);
    EXPECT_THROW(Syntax::commandValue("QT"), CommandStringException);
    EXPECT_THROW(Syntax::commandValue(" qt"), CommandStringException);
    EXPECT_THROW(Syntax::commandValue("qt "), CommandStringException);
}

TEST(test_syntax, test_commandCount)
{
    EXPECT_EQ(Syntax::commandCount(), 11);
}

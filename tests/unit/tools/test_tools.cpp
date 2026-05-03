#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <tools/stringlist.h>
#include <tools/tools.h>

TEST(tools, ScopeGuard)
{
    const std::string result{"A TEST!"};
    auto string = new std::string(result);

    {
        EXPECT_EQ(*string, result);

        Tools::ScopeGuard guard(
            [&]()
            {
                delete string;
                string = nullptr;
            });

        EXPECT_EQ(*string, result);
    }

    EXPECT_EQ(string, nullptr);
    EXPECT_EQ(result, "A TEST!");
}

TEST(tools, prepareIncludePath)
{
    const std::string result1{"a/b/c.h"};
    EXPECT_EQ(Tools::prepareIncludePath("a/b/c.h"), result1);
    EXPECT_EQ(Tools::prepareIncludePath("<a/b/c.h>"), result1);
    EXPECT_EQ(Tools::prepareIncludePath("\"a/b/c.h\""), result1);
    EXPECT_EQ(Tools::prepareIncludePath("<a/b/c.h\""), result1);
    EXPECT_EQ(Tools::prepareIncludePath("\"a/b/c.h>"), result1);
    EXPECT_NE(Tools::prepareIncludePath("\"<a/b/c.h>\""), result1);
    EXPECT_NE(Tools::prepareIncludePath("<\"a/b/c.h\""), result1);

    const std::string result2{"c.h"};
    EXPECT_EQ(Tools::prepareIncludePath("c.h"), result2);
    EXPECT_EQ(Tools::prepareIncludePath("\"c.h\""), result2);
    EXPECT_EQ(Tools::prepareIncludePath("<c.h>"), result2);
    EXPECT_EQ(Tools::prepareIncludePath("\"c.h>"), result2);
    EXPECT_EQ(Tools::prepareIncludePath("<c.h\""), result2);
    EXPECT_NE(Tools::prepareIncludePath("\"<c.h\""), result2);
    EXPECT_NE(Tools::prepareIncludePath("<\"c.h\""), result2);
}

TEST(tools, contains)
{
    const StringList vector{"ab", "cd", "ef", ""};
    EXPECT_TRUE(Tools::contains(vector, "ab"));
    EXPECT_TRUE(Tools::contains(vector, ""));
    EXPECT_FALSE(Tools::contains(vector, "abc"));
    EXPECT_FALSE(Tools::contains(vector, " "));
    EXPECT_FALSE(Tools::contains(vector, "AB"));
    EXPECT_FALSE(Tools::contains(vector, "a"));

    const std::string string{"abcd efgh"};
    EXPECT_TRUE(Tools::contains(string, "b"));
    EXPECT_TRUE(Tools::contains(string, "abc"));
    EXPECT_TRUE(Tools::contains(string, " "));
    EXPECT_TRUE(Tools::contains(string, "d efg"));
    EXPECT_FALSE(Tools::contains(string, "A"));
    EXPECT_FALSE(Tools::contains(string, "i"));
    EXPECT_FALSE(Tools::contains(string, "  "));
}

TEST(tools, listToString)
{
    const StringList vector{"ab", "cd", "ef", ""};

    EXPECT_EQ(Tools::listToString(vector), "ab, cd, ef");
    EXPECT_EQ(Tools::listToString({"", "", ""}), "");
    EXPECT_EQ(Tools::listToString({" ", " ", " "}), " ,  ,  ");
}

TEST(tools, inBrackets)
{
    EXPECT_EQ(Tools::inBrackets("abc"), "(abc)");
    EXPECT_EQ(Tools::inBrackets(" "), "( )");
    EXPECT_EQ(Tools::inBrackets(""), "()");
    EXPECT_EQ(Tools::inBrackets("(abc)"), "((abc))");
}

TEST(tools, boolToString)
{
    EXPECT_EQ(Tools::boolToString(true), "true");
    EXPECT_EQ(Tools::boolToString(false), "false");
    EXPECT_EQ(Tools::boolToString(123), "true");
    EXPECT_EQ(Tools::boolToString(-1), "true");
}

TEST(tools, isPathToFile)
{
    // TODO: this is platform-specific! Make separate tests for macOS and Windows.
    // Also, maybe just check the test executable path here?
    //EXPECT_TRUE(Tools::isPathToFile("/a/b/c.exe"));
#ifdef GTEST_OS_MAC
    EXPECT_TRUE(Tools::isPathToFile("/bin/sh"));
#elif defined(GTEST_OS_LINUX)
    EXPECT_TRUE(Tools::isPathToFile("/usr/bin/sh"));
#elif defined(GTEST_OS_WIN)
    // sth...
#else
    ASSERT_TRUE(false);
#endif

    EXPECT_FALSE(Tools::isPathToFile("/usr/bin/"));
    EXPECT_FALSE(Tools::isPathToFile("/usr/bin"));
    EXPECT_FALSE(Tools::isPathToFile("/a/b/"));
    EXPECT_FALSE(Tools::isPathToFile("/a/b"));
}

TEST(tools, isHeaderFile)
{
    EXPECT_TRUE(Tools::isHeaderFile("string.h"));
    EXPECT_TRUE(Tools::isHeaderFile("a.h"));
    EXPECT_TRUE(Tools::isHeaderFile("a.hpp"));
    EXPECT_TRUE(Tools::isHeaderFile("a.hxx"));
    EXPECT_TRUE(Tools::isHeaderFile("some/folder/a.h"));
    EXPECT_TRUE(Tools::isHeaderFile("some/folder/a.hpp"));
    EXPECT_TRUE(Tools::isHeaderFile("some/folder/a.hxx"));

    EXPECT_FALSE(Tools::isHeaderFile("a.c"));
    EXPECT_FALSE(Tools::isHeaderFile("a.cpp"));
    EXPECT_FALSE(Tools::isHeaderFile("a.cxx"));
    EXPECT_FALSE(Tools::isHeaderFile("some/folder/a.c"));
    EXPECT_FALSE(Tools::isHeaderFile("some/folder/a.cpp"));
    EXPECT_FALSE(Tools::isHeaderFile("some/folder/a.cxx"));
}

TEST(tools, isWhitespace)
{
    EXPECT_TRUE(Tools::isWhitespace(' '));
    EXPECT_TRUE(Tools::isWhitespace('\t'));
    EXPECT_TRUE(Tools::isWhitespace('\v'));
    EXPECT_FALSE(Tools::isWhitespace('a'));
    EXPECT_FALSE(Tools::isWhitespace('z'));
    EXPECT_FALSE(Tools::isWhitespace(','));
    EXPECT_FALSE(Tools::isWhitespace(0));
    EXPECT_FALSE(Tools::isWhitespace(-1));
    EXPECT_FALSE(Tools::isWhitespace(-127));
}

TEST(tools, inQuotes)
{
    EXPECT_EQ(Tools::inQuotes("abc"), "\"abc\"");
    EXPECT_EQ(Tools::inQuotes(" "), "\" \"");
    EXPECT_EQ(Tools::inQuotes(""), "\"\"");
    EXPECT_EQ(Tools::inQuotes("hello world"), "\"hello world\"");
    EXPECT_EQ(Tools::inQuotes("path/to/file.h"), "\"path/to/file.h\"");
}

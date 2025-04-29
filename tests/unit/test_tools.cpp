#include <gtest/gtest.h>

#include <tools/tools.h>

TEST(test_tools, testContains)
{
    const std::vector<std::string> vector{"ab", "cd", "ef"};
    EXPECT_TRUE(Tools::contains(vector, "ab"));
    EXPECT_FALSE(Tools::contains(vector, "abc"));
}

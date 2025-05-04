#include "parsing/syntax.h"
#include <gtest/gtest.h>

#include <project/command.h>

TEST(test_command, test_Command)
{
    Command c1;

    EXPECT_FALSE(c1.isValid());
    EXPECT_FALSE(c1.isReadyToExecute());
    EXPECT_EQ(c1.whole(), "");
    EXPECT_EQ(c1.value(), "");

    EXPECT_FALSE(c1.executable.isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.library.isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.object.isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.include.isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.option.isValid(Syntax::Command::Executable));

    c1.finalize();

    EXPECT_FALSE(c1.isValid());
    EXPECT_FALSE(c1.isReadyToExecute());
    EXPECT_EQ(c1.whole(), "");
    EXPECT_EQ(c1.value(), "");

    EXPECT_FALSE(c1.executable.isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.library.isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.object.isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.include.isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.option.isValid(Syntax::Command::Executable));
}

#include "tools/log.h"
#include <gtest/gtest.h>

#include <parsing/syntax.h>
#include <project/command.h>

TEST(test_command, test_Command)
{
    Command c1;

    EXPECT_FALSE(c1.isValid());
    EXPECT_FALSE(c1.isReadyToExecute());
    EXPECT_EQ(c1.whole(), "unknown");
    EXPECT_EQ(c1.value(), "");

    EXPECT_FALSE(c1.executable().isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.library().isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.object().isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.include().isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.option().isValid(Syntax::Command::Executable));

    c1.finalize();

    EXPECT_FALSE(c1.isValid());
    EXPECT_FALSE(c1.isReadyToExecute());
    EXPECT_EQ(c1.whole(), "unknown");
    EXPECT_EQ(c1.value(), "");

    EXPECT_FALSE(c1.executable().isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.library().isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.object().isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.include().isValid(Syntax::Command::Executable));
    EXPECT_FALSE(c1.option().isValid(Syntax::Command::Executable));
}

TEST(test_command, test_isValid)
{
    Log::setLogLevel(Log::Type::Verbose);

    {
        Command c;
        EXPECT_FALSE(c.isValid());
        EXPECT_FALSE(c.append("random string"));
        EXPECT_FALSE(c.isValid());
    }

    {
        Command c;
        EXPECT_FALSE(c.isValid());
        EXPECT_FALSE(c.append("incorrect"));
        EXPECT_FALSE(c.isValid());
    }

    {
        Command c;
        EXPECT_FALSE(c.isValid());
        EXPECT_TRUE(c.append("include"));
        EXPECT_FALSE(c.isValid());
    }

    {
        Command c;
        EXPECT_FALSE(c.isValid());
        EXPECT_TRUE(c.append("include"));
        EXPECT_TRUE(c.append("random.h"));
        EXPECT_TRUE(c.isValid());
    }

    {
        Command c;
        EXPECT_FALSE(c.isValid());
        EXPECT_TRUE(c.append("source"));
        EXPECT_TRUE(c.append("random.cpp"));
        EXPECT_TRUE(c.isValid());
    }

    {
        Command c;
        EXPECT_FALSE(c.isValid());
        EXPECT_TRUE(c.append("source"));
        // TODO: make it fail?
        EXPECT_TRUE(c.append(Syntax::Modifier::Dynamic));
        EXPECT_FALSE(c.append("random.cpp"));
        EXPECT_FALSE(c.isValid());
    }
}

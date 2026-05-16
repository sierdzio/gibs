#include <gtest/gtest.h>
#include <exceptions/commandexception.h>

#include <logger/log.h>
#include <parsing/syntax.h>
#include <project/command.h>

TEST(command, Command)
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

    c1.finalize({});

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

TEST(command, isValid)
{
    Log::setLogLevel(Log::Type::Silent);

    {
        Command c;
        EXPECT_FALSE(c.isValid());
    }

    {
        Command c;
        EXPECT_TRUE(c.append("invalid"));
        EXPECT_FALSE(c.isValid());
    }

    {
        Command c;
        EXPECT_TRUE(c.append("unknown"));
        EXPECT_FALSE(c.isValid());
    }

    {
        Command c;
        EXPECT_FALSE(c.append("random string"));
        EXPECT_FALSE(c.isValid());
    }

    {
        Command c;
        EXPECT_FALSE(c.append("incorrect"));
        EXPECT_FALSE(c.isValid());
    }

    {
        Command c;
        EXPECT_TRUE(c.append("include"));
        EXPECT_FALSE(c.isValid());
    }

    {
        Command c;
        EXPECT_TRUE(c.append("include"));
        EXPECT_TRUE(c.append("random.h"));
        EXPECT_TRUE(c.isValid());
    }

    {
        Command c;
        EXPECT_TRUE(c.append("source"));
        EXPECT_TRUE(c.append("random.cpp"));
        EXPECT_TRUE(c.isValid());
    }

    {
        Command c;
        EXPECT_TRUE(c.append("source"));
        // TODO: make it fail?
        EXPECT_TRUE(c.append(Syntax::Modifier::Dynamic));
        EXPECT_FALSE(c.append("random.cpp"));
        EXPECT_FALSE(c.isValid());
    }
}

TEST(command, OptionComponentDefine)
{
    Log::setLogLevel(Log::Type::Silent);

    // Test simple option name conversion
    {
        Command c;
        EXPECT_TRUE(c.append("feature"));
        EXPECT_TRUE(c.append("name"));
        EXPECT_TRUE(c.append("my-feature"));
        c.finalize({});
        EXPECT_EQ(c.option().define(), "MY_FEATURE");
    }

    // Test option with multiple dashes
    {
        Command c;
        EXPECT_TRUE(c.append("option"));
        EXPECT_TRUE(c.append("name"));
        EXPECT_TRUE(c.append("my-long-option"));
        c.finalize({});
        EXPECT_EQ(c.option().define(), "MY_LONG_OPTION");
    }

    // Test option without dashes
    {
        Command c;
        EXPECT_TRUE(c.append("feature"));
        EXPECT_TRUE(c.append("name"));
        EXPECT_TRUE(c.append("MYFEATURE"));
        c.finalize({});
        EXPECT_EQ(c.option().define(), "MYFEATURE");
    }

    // Test option with lowercase
    {
        Command c;
        EXPECT_TRUE(c.append("option"));
        EXPECT_TRUE(c.append("name"));
        EXPECT_TRUE(c.append("lowercase-option"));
        c.finalize({});
        EXPECT_EQ(c.option().define(), "LOWERCASE_OPTION");
    }
}

TEST(command, OptionFinalizeOverridesDefaultValue)
{
    Log::setLogLevel(Log::Type::Silent);

    ArgumentsList arguments;
    arguments["my-feature"] = true;

    Command c;
    EXPECT_TRUE(c.append("feature"));
    EXPECT_TRUE(c.append("name"));
    EXPECT_TRUE(c.append("my-feature"));
    EXPECT_TRUE(c.append("default"));
    EXPECT_TRUE(c.append("off"));
    c.finalize(arguments);

    EXPECT_TRUE(c.isValid());
    EXPECT_EQ(c.option().name, "my-feature");
    EXPECT_FALSE(c.option().defaultValue);
    EXPECT_TRUE(c.option().isOn);
}

TEST(command, OptionFinalizeUsesDefaultForNonBooleanOverride)
{
    Log::setLogLevel(Log::Type::Silent);

    ArgumentsList arguments;
    arguments["my-feature"] = std::string("not-a-bool");

    Command c;
    EXPECT_TRUE(c.append("feature"));
    EXPECT_TRUE(c.append("name"));
    EXPECT_TRUE(c.append("my-feature"));
    EXPECT_TRUE(c.append("default"));
    EXPECT_TRUE(c.append("on"));
    c.finalize(arguments);

    EXPECT_TRUE(c.isValid());
    EXPECT_EQ(c.option().name, "my-feature");
    EXPECT_TRUE(c.option().defaultValue);
    EXPECT_TRUE(c.option().isOn);
}

TEST(command, PathSemantics)
{
    {
        Command c;
        EXPECT_TRUE(c.append("source"));
        EXPECT_TRUE(c.append("main.cpp"));
        c.finalize({});

        EXPECT_TRUE(c.hasPath());
        EXPECT_EQ(c.path(), "main.o");
    }

    {
        Command c;
        EXPECT_TRUE(c.append("include"));
        EXPECT_TRUE(c.append("file.h"));
        c.finalize({});

        EXPECT_TRUE(c.hasPath());
        EXPECT_EQ(c.path(), "file.h");
    }

    {
        Command c;
        EXPECT_TRUE(c.append("executable"));
        EXPECT_TRUE(c.append("name"));
        EXPECT_TRUE(c.append("app"));
        c.finalize({});

        EXPECT_TRUE(c.hasPath());
        EXPECT_EQ(c.path(), "app");
    }

    {
        Command c;
        EXPECT_TRUE(c.append("library"));
        EXPECT_TRUE(c.append("type"));
        EXPECT_TRUE(c.append("static"));
        EXPECT_TRUE(c.append("name"));
        EXPECT_TRUE(c.append("mylib"));
        c.finalize({});

        EXPECT_TRUE(c.hasPath());
        EXPECT_EQ(c.path(), "mylib");
    }

    {
        Command c;
        EXPECT_TRUE(c.append("feature"));
        EXPECT_TRUE(c.append("name"));
        EXPECT_TRUE(c.append("my-feature"));

        EXPECT_FALSE(c.hasPath());
        EXPECT_THROW(c.path(), CommandStringException);
    }
}

TEST(command, ObjectComponentDefines)
{
    Log::setLogLevel(Log::Type::Silent);

    // Test that ObjectComponent has defines field
    {
        Command c;
        EXPECT_TRUE(c.append("source"));
        EXPECT_TRUE(c.append("test.cpp"));
        c.finalize({});

        // Initially empty
        EXPECT_TRUE(c.object().defines.empty());

        // Can add defines
        c.objectReference().defines.emplace_back("DEBUG");
        EXPECT_EQ(c.object().defines.size(), 1);
        EXPECT_EQ(c.object().defines[0], "DEBUG");

        // Can add multiple defines
        c.objectReference().defines.emplace_back("MY_FEATURE");
        EXPECT_EQ(c.object().defines.size(), 2);
        EXPECT_EQ(c.object().defines[1], "MY_FEATURE");
    }
}

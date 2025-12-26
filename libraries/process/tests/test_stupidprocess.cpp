#include <gtest/gtest.h>

#include <process/stupidprocess.h>

TEST(test_stupidprocess, test_defaults)
{
    StupidProcess process;

    EXPECT_TRUE(process.executable().empty());
    EXPECT_TRUE(process.arguments().empty());
    EXPECT_TRUE(process.metaInformation().empty());
    EXPECT_TRUE(process.executable().empty());
    EXPECT_EQ(process.result().rawCode, -1);
    EXPECT_EQ(process.result().status, Exit::Status::NotExecuted);
    EXPECT_FALSE(process.isFinished());

    process.setDuration(1ms);
    const auto result = process.start();
    std::this_thread::sleep_for(10ms);

    EXPECT_EQ(process.result().rawCode, 1);
    EXPECT_EQ(process.result().status, Exit::Status::FailedToExecute);
    EXPECT_FALSE(result);
}

TEST(test_stupidprocess, test_setExecutable)
{
    StupidProcess process;

    process.setExecutable("abc");

    EXPECT_EQ(process.executable(), "abc");
}

TEST(test_stupidprocess, test_setArguments)
{
    StupidProcess process;
    const std::vector<std::string> args {"a", "b", "cd"};

    process.setArguments(args);

    EXPECT_EQ(process.arguments(), args);
}

TEST(test_stupidprocess, test_setMetaInformation)
{
    StupidProcess process;

    process.setMetaInformation("abc");

    EXPECT_EQ(process.metaInformation(), "abc");
}

TEST(test_stupidprocess, test_setDuration)
{
    StupidProcess process;

    process.setDuration(123ms);

    EXPECT_EQ(process.duration(), 123ms);
}

TEST(test_stupidprocess, test_normalRun)
{
    StupidProcess process;
    process.setExecutable("abc");

    EXPECT_EQ(process.executable(), "abc");

    process.setDuration(1ms);
    const auto result = process.start();
    std::this_thread::sleep_for(10ms);

    EXPECT_EQ(process.result().rawCode, 0);
    EXPECT_EQ(process.result().status, Exit::Status::Success);

    EXPECT_TRUE(result);
}

TEST(test_stupidprocess, test_duration)
{
    StupidProcess process;
    process.setExecutable("echo");
    process.setDuration(2ms);
    const auto result = process.start();
    std::this_thread::sleep_for(1ms); // Too short

    EXPECT_EQ(process.result().rawCode, -1);
    EXPECT_EQ(process.result().status, Exit::Status::InProgress);

    EXPECT_TRUE(result);
}


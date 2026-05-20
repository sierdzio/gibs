#include <gtest/gtest.h>

#include <process/stupidprocess.h>

TEST(stupidprocess, defaults)
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

TEST(stupidprocess, setExecutable)
{
    StupidProcess process;

    process.setExecutable("abc");

    EXPECT_EQ(process.executable(), "abc");
}

TEST(stupidprocess, setArguments)
{
    StupidProcess process;
    const std::vector<std::string> args{"a", "b", "cd"};

    process.setArguments(args);

    EXPECT_EQ(process.arguments(), args);
}

TEST(stupidprocess, setMetaInformation)
{
    StupidProcess process;

    process.setMetaInformation("abc");

    EXPECT_EQ(process.metaInformation(), "abc");
}

TEST(stupidprocess, setDuration)
{
    StupidProcess process;

    process.setDuration(123ms);

    EXPECT_EQ(process.duration(), 123ms);
}

TEST(stupidprocess, normalRun)
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

TEST(stupidprocess, duration)
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

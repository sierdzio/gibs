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

    EXPECT_EQ(process.result().rawCode, 0);
    EXPECT_EQ(process.result().status, Exit::Status::Success);

    EXPECT_TRUE(result);
}

TEST(test_stupidprocess, test_duration)
{
    StupidProcess process;
    process.setDuration(2ms);
    const auto result = process.start();
    std::this_thread::sleep_for(1ms); // Too short

    EXPECT_EQ(process.result().rawCode, -1);
    EXPECT_EQ(process.result().status, Exit::Status::InProgress);

    EXPECT_TRUE(result);
}


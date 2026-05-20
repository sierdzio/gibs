#include <gtest/gtest.h>

#include <process/systemprocess.h>

using namespace std::chrono_literals;

TEST(systemprocess, defaults)
{
    SystemProcess process;

    EXPECT_TRUE(process.executable().empty());
    EXPECT_TRUE(process.arguments().empty());
    EXPECT_TRUE(process.metaInformation().empty());
    EXPECT_TRUE(process.executable().empty());
    EXPECT_EQ(process.result().rawCode, -1);
    EXPECT_EQ(process.result().status, Exit::Status::NotExecuted);
    EXPECT_FALSE(process.isFinished());

    const auto result = process.start();
    std::this_thread::sleep_for(10ms);

    ASSERT_FALSE(result);
    EXPECT_EQ(process.result().rawCode, 1);
    EXPECT_EQ(process.result().status, Exit::Status::FailedToExecute);
}

TEST(systemprocess, printf)
{
    SystemProcess process;
    process.setExecutable("printf");
    process.setArguments({"\"hello there!\""});

    const auto result = process.start();
    std::this_thread::sleep_for(800ms);

    ASSERT_TRUE(result);
    ASSERT_TRUE(process.isFinished());
    EXPECT_EQ(process.result().rawCode, 0);
    EXPECT_EQ(process.result().status, Exit::Status::Success);
}

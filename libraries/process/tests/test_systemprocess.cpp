#include <gtest/gtest.h>

#include <process/systemprocess.h>

using namespace std::chrono_literals;

TEST(test_systemprocess, test_defaults)
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

    EXPECT_EQ(process.result().rawCode, 1);
    EXPECT_EQ(process.result().status, Exit::Status::FailedToExecute);
    EXPECT_FALSE(result);
}

TEST(test_systemprocess, test_echo)
{
    SystemProcess process;
    process.setExecutable("echo");
    process.setArguments({"\"hello there!\""});

    const auto result = process.start();
    std::this_thread::sleep_for(500ms);

    EXPECT_EQ(process.result().rawCode, 0);
    EXPECT_EQ(process.result().status, Exit::Status::Success);
    EXPECT_TRUE(result);
}

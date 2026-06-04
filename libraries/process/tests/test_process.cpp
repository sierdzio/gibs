#include <gtest/gtest.h>

#include <process/process.h>

#include <thread>

TEST(process, executeCommandOnLinux)
{
#if defined(__linux__)
    Process process;
    process.setExecutable("printf");
    process.setArguments({"hello"});

    const auto result = process.start();
    ASSERT_TRUE(result);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_TRUE(process.isFinished());
    EXPECT_EQ(process.result().rawCode, 0);
    EXPECT_EQ(process.result().status, Exit::Status::Success);
#else
    GTEST_SKIP() << "Linux process execution is only tested on Linux.";
#endif
}

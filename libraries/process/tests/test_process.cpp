#include <gtest/gtest.h>

#include <process/process.h>

#include <chrono>
#include <thread>

static bool waitForProcessFinish(Process &process)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (process.isFinished())
        {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return process.isFinished();
}

TEST(process, executeCommandOnUnix)
{
#if defined(__unix__) || defined(__APPLE__)
    Process process;
    process.setExecutable("printf");
    process.setArguments({"hello"});

    const auto result = process.start();
    ASSERT_TRUE(result);
    ASSERT_TRUE(waitForProcessFinish(process));

    EXPECT_EQ(process.result().rawCode, 0);
    EXPECT_EQ(process.result().status, Exit::Status::Success);
#else
    GTEST_SKIP() << "Unix process execution is only tested on Unix-like platforms.";
#endif
}

TEST(process, captureOutputWhenLoggingEnabled)
{
#if defined(__unix__) || defined(__APPLE__)
    Process process;
    process.setExecutable("printf");
    process.setArguments({"hello"});
    process.setLogProcessOutput(true);

    testing::internal::CaptureStdout();
    const auto result = process.start();
    ASSERT_TRUE(result);
    ASSERT_TRUE(waitForProcessFinish(process));
    const auto output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(process.result().rawCode, 0);
    EXPECT_EQ(process.result().status, Exit::Status::Success);
    EXPECT_EQ(output, "hello");
#else
    GTEST_SKIP() << "Unix process execution is only tested on Unix-like platforms.";
#endif
}

TEST(process, suppressOutputWhenLoggingDisabled)
{
#if defined(__unix__) || defined(__APPLE__)
    Process process;
    process.setExecutable("printf");
    process.setArguments({"hello"});
    process.setLogProcessOutput(false);

    testing::internal::CaptureStdout();
    const auto result = process.start();
    ASSERT_TRUE(result);
    ASSERT_TRUE(waitForProcessFinish(process));
    const auto output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(process.result().rawCode, 0);
    EXPECT_EQ(process.result().status, Exit::Status::Success);
    EXPECT_EQ(output, "");
#else
    GTEST_SKIP() << "Unix process execution is only tested on Unix-like platforms.";
#endif
}

TEST(process, logBufferedOutputOnProcessFailureWhenLoggingDisabled)
{
#if defined(__unix__) || defined(__APPLE__)
    Process process;
    process.setExecutable("sh");
    process.setArguments({"-c", "echo error >&2; exit 42"});
    process.setLogProcessOutput(false);

    testing::internal::CaptureStderr();
    const auto result = process.start();
    ASSERT_TRUE(result);
    ASSERT_TRUE(waitForProcessFinish(process));
    const auto output = testing::internal::GetCapturedStderr();

    EXPECT_EQ(process.result().rawCode, 42);
    EXPECT_EQ(process.result().status, Exit::Status::FailedDuringExecution);
    EXPECT_NE(output.find("error"), std::string::npos);
#else
    GTEST_SKIP() << "Unix process execution is only tested on Unix-like platforms.";
#endif
}

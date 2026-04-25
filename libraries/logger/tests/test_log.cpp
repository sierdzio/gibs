#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include <logger/exceptions/loglevelexception.h>
#include <logger/log.h>

TEST(log, typeString)
{
    EXPECT_EQ(Log::typeString(Log::Type::Silent), "silent");
    EXPECT_EQ(Log::typeString(Log::Type::Error), "error");
    EXPECT_EQ(Log::typeString(Log::Type::Warning), "warning");
    EXPECT_EQ(Log::typeString(Log::Type::Information), "information");
    EXPECT_EQ(Log::typeString(Log::Type::Debug), "debug");
    EXPECT_EQ(Log::typeString(Log::Type::Verbose), "verbose");

    EXPECT_THROW(Log::typeString(static_cast<Log::Type>(123)), LogLevelException);
}

TEST(log, typeValue)
{
    EXPECT_EQ(Log::typeValue("silent"), Log::Type::Silent);
    EXPECT_EQ(Log::typeValue("error"), Log::Type::Error);
    EXPECT_EQ(Log::typeValue("warning"), Log::Type::Warning);
    EXPECT_EQ(Log::typeValue("information"), Log::Type::Information);
    EXPECT_EQ(Log::typeValue("debug"), Log::Type::Debug);
    EXPECT_EQ(Log::typeValue("verbose"), Log::Type::Verbose);

    EXPECT_NE(Log::typeValue("Silent"), Log::Type::Silent);
    EXPECT_NE(Log::typeValue(" silent"), Log::Type::Silent);
    EXPECT_NE(Log::typeValue("silent "), Log::Type::Silent);

    EXPECT_EQ(Log::typeValue("siLent"), Log::Type::Information);
    EXPECT_EQ(Log::typeValue("errOr"), Log::Type::Information);
    EXPECT_EQ(Log::typeValue("warNing"), Log::Type::Information);
    EXPECT_EQ(Log::typeValue("informatIon"), Log::Type::Information);
    EXPECT_EQ(Log::typeValue("debUg"), Log::Type::Information);
    EXPECT_EQ(Log::typeValue("verBose"), Log::Type::Information);
}

TEST(log, LogLevel)
{
    EXPECT_TRUE(Log::logLevelsCount() > 0);

    constexpr auto defaultLevel = Log::Type::Verbose;
    EXPECT_EQ(Log::logLevel(), defaultLevel);
    EXPECT_TRUE(Log::isWithinLogLevel(Log::Type::Silent));
    EXPECT_TRUE(Log::isWithinLogLevel(Log::Type::Error));
    EXPECT_TRUE(Log::isWithinLogLevel(Log::Type::Warning));
    EXPECT_TRUE(Log::isWithinLogLevel(defaultLevel));

    Log::setLogLevel(Log::Type::Silent);
    EXPECT_EQ(Log::logLevel(), Log::Type::Silent);
    EXPECT_TRUE(Log::isWithinLogLevel(Log::Type::Silent));

    Log::setLogLevel(Log::Type::Verbose);
    EXPECT_EQ(Log::logLevel(), Log::Type::Verbose);
    EXPECT_TRUE(Log::isWithinLogLevel(Log::Type::Silent));
    EXPECT_TRUE(Log::isWithinLogLevel(Log::Type::Debug));
    EXPECT_TRUE(Log::isWithinLogLevel(Log::Type::Verbose));

    EXPECT_THROW(Log::setLogLevel(static_cast<Log::Type>(123)), LogLevelException);

    // Go back to default:
    Log::setLogLevel(defaultLevel);
    EXPECT_EQ(Log::logLevel(), defaultLevel);
}

TEST(log, UseColorfulLogs)
{
    EXPECT_TRUE(Log::usingColorfulLogs());
    Log::setUseColorfulLogs(false);
    EXPECT_FALSE(Log::usingColorfulLogs());
    Log::setUseColorfulLogs(true);
}

TEST(log, LogFileDuplication)
{
    namespace fs = std::filesystem;
    const auto tempLogFile = fs::temp_directory_path() / "logger_test_output.log";

    Log::setUseColorfulLogs(false);
    Log::setLogFile(tempLogFile.string());

    std::ostringstream capturedOutput;
    auto *previousBuffer = std::cout.rdbuf(capturedOutput.rdbuf());

    Log::information("Duplicated", "log", "message");
    std::cout.rdbuf(previousBuffer);

    Log::closeLogFile();

    std::ifstream file(tempLogFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    const std::string fileContents{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };
    file.close();

    EXPECT_EQ(capturedOutput.str(), fileContents);
    EXPECT_TRUE(fs::remove(tempLogFile));
}

TEST(log, LogFileInvalidPath)
{
    namespace fs = std::filesystem;
    const auto invalidPath = "/nonexistent/directory/that/does/not/exist/logfile.log";

    EXPECT_THROW(Log::setLogFile(invalidPath), std::runtime_error);
}

TEST(log, LogFileInvalidPathConsoleStillWorks)
{
    namespace fs = std::filesystem;
    const auto invalidPath = "/nonexistent/directory/that/does/not/exist/logfile.log";

    Log::setUseColorfulLogs(false);

    try
    {
        Log::setLogFile(invalidPath);
    }
    catch (const std::runtime_error &)
    {
        // Expected to fail
    }

    std::ostringstream capturedOutput;
    auto *previousBuffer = std::cout.rdbuf(capturedOutput.rdbuf());

    Log::information("Console", "still", "works");
    std::cout.rdbuf(previousBuffer);

    EXPECT_FALSE(capturedOutput.str().empty());
    EXPECT_TRUE(capturedOutput.str().find("Console") != std::string::npos);
}

TEST(log, Begining)
{
    EXPECT_TRUE(Log::Private::beginning(Log::Type::Information).size() > 0);
}

TEST(log, Ending)
{
    EXPECT_TRUE(Log::Private::ending(Log::Type::Information).size() > 0);
}

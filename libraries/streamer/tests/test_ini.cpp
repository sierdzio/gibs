#include <gtest/gtest.h>

#include <streamer/ini.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <type_traits>

struct DerivedObject : Object
{
};

struct UnrelatedType
{
};

template <typename Type, typename = void> struct IsValidIni : std::false_type
{
};

template <typename Type>
struct IsValidIni<Type, std::void_t<Ini<Type>>> : std::true_type
{
};

namespace
{
std::string readFileContents(const std::filesystem::path &path)
{
    std::ifstream input(path);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
} // namespace

TEST(IniTest, TemplateIsRestrictedToObjectTypes)
{
    EXPECT_TRUE(IsValidIni<Object>::value);
    EXPECT_TRUE(IsValidIni<DerivedObject>::value);
    EXPECT_FALSE(IsValidIni<Value>::value);
    EXPECT_FALSE(IsValidIni<UnrelatedType>::value);
    EXPECT_FALSE(IsValidIni<int>::value);
}

TEST(IniTest, WriteValueSerializesScalarAndCompositeValues)
{
    std::ostringstream output;
    Ini<Object>::writeValue(output, Value{"hello"}, false);
    EXPECT_EQ(output.str(), "\"hello\"");

    output.str("");
    output.clear();
    Ini<Object>::writeValue(output, Value{42}, false);
    EXPECT_EQ(output.str(), "42");

    output.str("");
    output.clear();
    Ini<Object>::writeValue(output, Value{Array{Value{1}, Value{2}}}, false);
    EXPECT_EQ(output.str(), "[1,2]");

    output.str("");
    output.clear();
    Ini<Object>::writeValue(output, Value{Object{{"host", Value{"localhost"}}}}, false);
    EXPECT_EQ(output.str(), "\n\"host\" = \"localhost\"\n");
}

TEST(IniTest, SerializeReturnsDeterministicOutputForSingleValueCollections)
{
    EXPECT_EQ(Ini<Object>::serialize(Object{{"message", Value{"hello"}}}),
              "{\n\"message\": \"hello\"\n}\n");
    EXPECT_EQ(Ini<Object>::serialize(Array{Value{1}, Value{2}}), "[1,2]\n");
}

TEST(IniTest, IniWriteWritesSectionedConfigurationToDisk)
{
    const auto directory = std::filesystem::temp_directory_path() / "gibs-ini-tests";
    std::filesystem::create_directories(directory);

    Ini<Object> config;
    config.insert("database", Value{Object{{"host", Value{"localhost"}}}});

    const auto path = directory / "config.ini";
    EXPECT_TRUE(config.write(path));
    EXPECT_EQ(readFileContents(path), "[\"database\"]\n\n\"host\" = \"localhost\"\n\n");

    std::filesystem::remove_all(directory);
}

TEST(IniTest, IniWriteFailsWhenTheTargetPathIsNotWritable)
{
    const auto directory = std::filesystem::temp_directory_path() / "gibs-ini-invalid";
    std::filesystem::create_directories(directory);

    Ini<Object> config;
    config.insert("value", Value{1});

    EXPECT_FALSE(config.write(directory));

    std::filesystem::remove_all(directory);
}

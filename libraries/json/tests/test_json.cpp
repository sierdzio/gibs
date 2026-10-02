#include <gtest/gtest.h>

#include <json/json.h>

#include <filesystem>
#include <fstream>
#include <type_traits>

struct DerivedObject : Object
{
};

struct DerivedArray : Array
{
};

struct UnrelatedType
{
};

template <typename Type, typename = void> struct IsValidJson : std::false_type
{
};

template <typename Type>
struct IsValidJson<Type, std::void_t<Json<Type>>> : std::true_type
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

TEST(JsonTest, TemplateIsRestrictedToObjectOrArrayTypes)
{
    EXPECT_TRUE(IsValidJson<Object>::value);
    EXPECT_TRUE(IsValidJson<Array>::value);
    EXPECT_TRUE(IsValidJson<DerivedObject>::value);
    EXPECT_TRUE(IsValidJson<DerivedArray>::value);
    EXPECT_FALSE(IsValidJson<Value>::value);
    EXPECT_FALSE(IsValidJson<UnrelatedType>::value);
    EXPECT_FALSE(IsValidJson<int>::value);
}

TEST(JsonTest, ValueSupportsAllBuiltInJsonScalarAndCompositeTypes)
{
    EXPECT_EQ(Value{"hello"}.toString(), "\"hello\"");
    EXPECT_EQ(Value{std::string{"world"}}.toString(), "\"world\"");
    EXPECT_EQ(Value{42}.toString(), "42");
    EXPECT_EQ(Value{3.5}.toString(), "3.500000");
    EXPECT_EQ(Value{true}.toString(), "true");
    EXPECT_EQ(Value{false}.toString(), "false");

    const Value objectValue{Object{{"answer", Value{42}}}};
    EXPECT_EQ(objectValue.toString(), "{\n\"answer\": 42\n}\n");

    const Value arrayValue{Array{Value{1}, Value{2}}};
    EXPECT_EQ(arrayValue.toString(), "[1,2]\n");
}

TEST(JsonTest, CompositeValuesOwnTheirContentsByValue)
{
    Array sourceArray{Value{1}};
    Value arrayValue{sourceArray};
    auto &storedArray = std::get<Value::ArrayStorage>(arrayValue.data);
    storedArray.emplace_back(2);
    EXPECT_EQ(arrayValue.toString(), "[1,2]\n");
    EXPECT_EQ(sourceArray.elements.size(), 1u);

    Object sourceObject{{"array", Value{Array{Value{3}}}}};
    Value objectValue{sourceObject};
    sourceObject.insert("later", Value{4});
    EXPECT_EQ(objectValue.toString(), "{\n\"array\": [3]\n}\n");
}

TEST(JsonTest, ObjectAndArraySerializeTheirCollectionContents)
{
    const Object object{{"message", Value{"hello"}}};
    EXPECT_EQ(Object::serialize(object), "{\n\"message\": \"hello\"\n}\n");

    const Array array{Value{1}, Value{"two"}, Value{true}};
    EXPECT_EQ(Array::serialize(array), "[1,\"two\",true]\n");

    const Array objects{Value{Object{{"first", Value{1}}}},
                        Value{Object{{"second", Value{2}}}}};
    EXPECT_EQ(Array::serialize(objects),
              "[{\n\"first\": 1\n},{\n\"second\": 2\n}]\n");
}

TEST(JsonTest, NestedCompositeSeparatorsStayOnThePreviousLine)
{
    Value object;
    object.data = Value::ObjectStorage{
        {"nested", Value{Object{{"value", Value{1}}}}}, {"next", Value{2}}};

    EXPECT_EQ(object.toString(),
              "{\n\"nested\": {\n\"value\": 1\n},\n\"next\": 2\n}\n");
}

TEST(JsonTest, StringEscapingIsHandledCorrectly)
{
    const Value value{Object{{"message",
                             Value{"quote \"hi\"\nnext\tend\\slash"}}}};

    EXPECT_EQ(value.toString(),
              "{\n\"message\": \"quote \\\"hi\\\"\\nnext\\tend\\\\slash\"\n}\n");
}

TEST(JsonTest, InsertAndAppendAddValuesToCollections)
{
    Object object;
    object.insert("first", Value{1});
    object.insert("second", Value{"two"});

    EXPECT_EQ(object.elements.size(), 2u);
    EXPECT_TRUE(object.elements.contains("first"));
    EXPECT_TRUE(object.elements.contains("second"));

    Array array;
    array.append(Value{1});
    array.append(Value{2});

    EXPECT_EQ(array.elements.size(), 2u);
    EXPECT_EQ(Array::serialize(array), "[1,2]\n");
}

TEST(JsonTest, InsertAndAppendMoveTemporaryValues)
{
    Array nestedArray{Value{1}, Value{2}};
    Value arrayValue{std::move(nestedArray)};

    Object object;
    object.insert("nested", std::move(arrayValue));
    EXPECT_EQ(Object::serialize(object), "{\n\"nested\": [1,2]\n}\n");

    Value objectValue{Object{{"answer", Value{42}}}};
    Array array;
    array.append(std::move(objectValue));
    EXPECT_EQ(Array::serialize(array), "[{\n\"answer\": 42\n}]\n");
}

TEST(JsonTest, JsonWriteWritesObjectAndArrayContentToDisk)
{
    const auto directory = std::filesystem::temp_directory_path() / "gibs-json-tests";
    std::filesystem::create_directories(directory);

    Json<Object> object;
    object.insert("number", Value{42});

    const auto objectPath = directory / "object.json";
    EXPECT_TRUE(object.write(objectPath));
    EXPECT_EQ(readFileContents(objectPath), "{\n\"number\": 42\n}\n");

    Json<Array> array;
    array.append(Value{1});
    array.append(Value{2});

    const auto arrayPath = directory / "array.json";
    EXPECT_TRUE(array.write(arrayPath));
    EXPECT_EQ(readFileContents(arrayPath), "[\n1,\n2\n]\n");

    Json<Array> objectArray;
    objectArray.append(Value{Object{{"first", Value{1}}}});
    objectArray.append(Value{Object{{"second", Value{2}}}});

    const auto objectArrayPath = directory / "object-array.json";
    EXPECT_TRUE(objectArray.write(objectArrayPath));
    EXPECT_EQ(readFileContents(objectArrayPath),
              "[\n{\n\"first\": 1\n},\n{\n\"second\": 2\n}\n]\n");

    std::filesystem::remove_all(directory);
}

TEST(JsonTest, JsonWriteFailsWhenTheTargetPathIsNotWritable)
{
    const auto directory = std::filesystem::temp_directory_path() / "gibs-json-invalid";
    std::filesystem::create_directories(directory);

    Json<Object> object;
    object.insert("value", Value{1});

    EXPECT_FALSE(object.write(directory));

    std::filesystem::remove_all(directory);
}

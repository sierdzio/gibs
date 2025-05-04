#include <gtest/gtest.h>

#include <exceptions/targetidtypeexception.h>
#include <project/targetid.h>

TEST(test_targetid, test_typeString)
{
    EXPECT_EQ(TargetId::typeString(TargetId::Type::Unknown), "unknown");
    EXPECT_EQ(TargetId::typeString(TargetId::Type::Executable), "executable");
    EXPECT_EQ(TargetId::typeString(TargetId::Type::Library), "library");
    EXPECT_THROW(TargetId::typeString(static_cast<TargetId::Type>(123)),
                 TargetIdTypeException);
}

TEST(test_targetid, test_typeValue)
{
    EXPECT_EQ(TargetId::typeValue("unknown"), TargetId::Type::Unknown);
    EXPECT_EQ(TargetId::typeValue("executable"), TargetId::Type::Executable);
    EXPECT_EQ(TargetId::typeValue("library"), TargetId::Type::Library);

    EXPECT_EQ(TargetId::typeValue("abc"), TargetId::Type::Unknown);
    EXPECT_EQ(TargetId::typeValue("LIBRARY"), TargetId::Type::Unknown);
    EXPECT_EQ(TargetId::typeValue("Library"), TargetId::Type::Unknown);
}

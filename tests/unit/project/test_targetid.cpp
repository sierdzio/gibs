#include <gtest/gtest.h>

#include <exceptions/targetidtypeexception.h>
#include <project/targetid.h>

TEST(targetid, typeString)
{
    EXPECT_EQ(TargetId::typeString(TargetId::Type::Unknown), "unknown");
    EXPECT_EQ(TargetId::typeString(TargetId::Type::Executable), "executable");
    EXPECT_EQ(TargetId::typeString(TargetId::Type::Library), "library");
    EXPECT_THROW(TargetId::typeString(static_cast<TargetId::Type>(123)),
                 TargetIdTypeException);
}

TEST(targetid, typeValue)
{
    EXPECT_EQ(TargetId::typeValue("unknown"), TargetId::Type::Unknown);
    EXPECT_EQ(TargetId::typeValue("executable"), TargetId::Type::Executable);
    EXPECT_EQ(TargetId::typeValue("library"), TargetId::Type::Library);

    EXPECT_EQ(TargetId::typeValue("abc"), TargetId::Type::Unknown);
    EXPECT_EQ(TargetId::typeValue("LIBRARY"), TargetId::Type::Unknown);
    EXPECT_EQ(TargetId::typeValue("Library"), TargetId::Type::Unknown);
}

TEST(targetid, TargetId)
{
    const TargetId t1;
    const TargetId t2;

    EXPECT_EQ(t1, t2);
    EXPECT_TRUE(t1.isNull());
    EXPECT_TRUE(t2.isNull());
    EXPECT_EQ(t1.name(), "");
    EXPECT_EQ(t1.type(), TargetId::Type::Unknown);

    const TargetId t3("abc", TargetId::Type::Library);

    EXPECT_NE(t3, t1);
    EXPECT_NE(t3, t2);
    EXPECT_FALSE(t3.isNull());
    EXPECT_EQ(t3.name(), "abc");
    EXPECT_EQ(t3.type(), TargetId::Type::Library);

    const TargetId t4("abc", TargetId::Type::Library);

    EXPECT_NE(t4, t3);
    EXPECT_NE(t4, t2);
    EXPECT_NE(t4, t1);
    EXPECT_FALSE(t4.isNull());
    EXPECT_EQ(t4.name(), "abc");
    EXPECT_EQ(t4.type(), TargetId::Type::Library);
}

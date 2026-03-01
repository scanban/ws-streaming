#include <optional>
#include <string>
#include <type_traits>
#include <limits>

#include <gtest/gtest.h>

#include <ws-streaming/detail/semver.hpp>

using namespace testing;
using namespace wss::detail;

TEST(SemVerTest, DefaultConstructor)
{
    semver ver;

    EXPECT_EQ(ver.major(), 0);
    EXPECT_EQ(ver.minor(), 0);
    EXPECT_EQ(ver.revision(), 0);
}

TEST(SemVerTest, ExplicitConstructor)
{
    semver ver(1, 2, 3);

    EXPECT_EQ(ver.major(), 1);
    EXPECT_EQ(ver.minor(), 2);
    EXPECT_EQ(ver.revision(), 3);
}

TEST(SemVerTest, TryParse)
{
    EXPECT_EQ(semver::try_parse(""), std::nullopt);
    EXPECT_EQ(semver::try_parse("1"), std::nullopt);
    EXPECT_EQ(semver::try_parse("1.2"), std::nullopt);
    EXPECT_EQ(semver::try_parse("1.2.3x"), std::nullopt);
    EXPECT_EQ(semver::try_parse("1.2.x3"), std::nullopt);
    EXPECT_EQ(semver::try_parse("1.2x.3"), std::nullopt);
    EXPECT_EQ(semver::try_parse("1.x2.3"), std::nullopt);
    EXPECT_EQ(semver::try_parse("1x.2.3"), std::nullopt);
    EXPECT_EQ(semver::try_parse("x1.2.3"), std::nullopt);

    EXPECT_EQ(semver::try_parse("1.2.3"), semver(1, 2, 3));
}

TEST(SemVerTest, TryParseBoundaryAndMalformedInputs)
{
    constexpr auto max_value = std::numeric_limits<unsigned>::max();
    const std::string max_str = std::to_string(max_value);
    const std::string overflow_str = std::to_string(static_cast<unsigned long long>(max_value) + 1ULL);

    EXPECT_EQ(semver::try_parse("0.0.0"), semver(0, 0, 0));
    EXPECT_EQ(semver::try_parse(max_str + "." + max_str + "." + max_str), semver(max_value, max_value, max_value));

    EXPECT_EQ(semver::try_parse(overflow_str + ".0.0"), std::nullopt);
    EXPECT_EQ(semver::try_parse("0." + overflow_str + ".0"), std::nullopt);
    EXPECT_EQ(semver::try_parse("0.0." + overflow_str), std::nullopt);

    EXPECT_EQ(semver::try_parse("1..3"), std::nullopt);
    EXPECT_EQ(semver::try_parse(".2.3"), std::nullopt);
    EXPECT_EQ(semver::try_parse("1.2."), std::nullopt);
    EXPECT_EQ(semver::try_parse("1.2.3.4"), std::nullopt);
    EXPECT_EQ(semver::try_parse("-1.2.3"), std::nullopt);
}

TEST(SemVerTest, Compare)
{
    EXPECT_GE(semver(2, 0, 0), semver(1, 0, 0));
    EXPECT_GE(semver(2, 2, 0), semver(2, 1, 0));
    EXPECT_GE(semver(2, 2, 2), semver(2, 2, 1));

    EXPECT_LE(semver(1, 0, 0), semver(2, 0, 0));
    EXPECT_LE(semver(2, 1, 0), semver(2, 2, 0));
    EXPECT_LE(semver(2, 2, 1), semver(2, 2, 2));

    EXPECT_NE(semver(1, 1, 1), semver(1, 1, 2));
    EXPECT_NE(semver(1, 1, 1), semver(1, 2, 1));
    EXPECT_NE(semver(1, 1, 1), semver(2, 1, 1));
}

TEST(SemVerTest, CompareStrictBranches)
{
    EXPECT_EQ(semver(1, 2, 3), semver(1, 2, 3));

    EXPECT_LT(semver(1, 0, 0), semver(2, 0, 0));
    EXPECT_FALSE(semver(2, 0, 0) < semver(1, 0, 0));

    EXPECT_LT(semver(1, 1, 0), semver(1, 2, 0));
    EXPECT_FALSE(semver(1, 2, 0) < semver(1, 1, 0));

    EXPECT_LT(semver(1, 2, 0), semver(1, 2, 1));
    EXPECT_FALSE(semver(1, 2, 1) < semver(1, 2, 0));

    EXPECT_GT(semver(1, 2, 4), semver(1, 2, 3));
    EXPECT_FALSE(semver(1, 2, 3) > semver(1, 2, 3));
}

TEST(SemVerTest, TypeTraitsCopyAndMove)
{
    static_assert(std::is_copy_constructible_v<semver>);
    static_assert(std::is_copy_assignable_v<semver>);
    static_assert(std::is_move_constructible_v<semver>);
    static_assert(std::is_move_assignable_v<semver>);
    static_assert(std::is_nothrow_move_constructible_v<semver>);
    static_assert(std::is_nothrow_move_assignable_v<semver>);
}

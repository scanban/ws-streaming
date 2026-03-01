#include <cstdint>
#include <limits>
#include <optional>

#include <boost/endian/conversion.hpp>

#include <gtest/gtest.h>

#include <ws-streaming/metadata.hpp>
#include <ws-streaming/metadata_builder.hpp>
#include <ws-streaming/detail/linear_table.hpp>
#include <ws-streaming/detail/streaming_protocol.hpp>

namespace
{
    wss::metadata make_linear_metadata(
        std::int64_t start,
        std::int64_t delta,
        std::optional<std::int64_t> value_index = 0)
    {
        auto json = wss::metadata_builder("linear").linear_rule(start, delta).build();

        if (value_index.has_value())
            json["valueIndex"] = value_index.value();
        else
            json.erase("valueIndex");

        return wss::metadata(json);
    }
}

TEST(LinearTableTest, InitializesFromMetadataAndComputesValues)
{
    wss::detail::linear_table table(make_linear_metadata(10, 3, 4));

    EXPECT_EQ(table.driven_index(), 4);
    EXPECT_EQ(table.driven_value(), 10);
    EXPECT_EQ(table.value_at(6), 16);
}

TEST(LinearTableTest, UpdateWithoutValueIndexPreservesIndices)
{
    wss::detail::linear_table table(make_linear_metadata(0, 1, 2));
    table.drive_to(5);

    table.update(make_linear_metadata(100, 2, std::nullopt));

    EXPECT_EQ(table.driven_index(), 5);
    EXPECT_EQ(table.driven_value(), 106);
}

TEST(LinearTableTest, UpdateWithValueIndexResetsDrivenIndex)
{
    wss::detail::linear_table table(make_linear_metadata(7, 1, 1));
    table.drive_to(9);

    table.update(make_linear_metadata(20, 4, 5));

    EXPECT_EQ(table.driven_index(), 5);
    EXPECT_EQ(table.driven_value(), 20);
    EXPECT_EQ(table.value_at(7), 28);
}

TEST(LinearTableTest, PayloadUpdateDecodesLittleEndianFields)
{
    wss::detail::linear_table table(make_linear_metadata(0, 2, 0));

    wss::detail::streaming_protocol::linear_payload payload { };
    payload.sample_index = boost::endian::native_to_little<std::int64_t>(42);
    payload.value = boost::endian::native_to_little<std::int64_t>(1000);

    table.update(payload);

    EXPECT_EQ(table.driven_index(), 42);
    EXPECT_EQ(table.driven_value(), 1000);
    EXPECT_EQ(table.value_at(44), 1004);
}

TEST(LinearTableTest, SetAndDriveToTrackDrivenState)
{
    wss::detail::linear_table table(make_linear_metadata(0, 3, 0));

    table.set(10, 50);
    table.drive_to(12);

    EXPECT_EQ(table.driven_index(), 12);
    EXPECT_EQ(table.driven_value(), 56);
}

TEST(LinearTableTest, ValueAtSaturatesOnPositiveOverflow)
{
    wss::detail::linear_table table(make_linear_metadata(0, std::numeric_limits<std::int64_t>::max(), 0));

    EXPECT_EQ(table.value_at(2), std::numeric_limits<std::int64_t>::max());
}

TEST(LinearTableTest, ValueAtSaturatesOnNegativeOverflow)
{
    wss::detail::linear_table table(make_linear_metadata(0, std::numeric_limits<std::int64_t>::min(), 0));

    EXPECT_EQ(table.value_at(2), std::numeric_limits<std::int64_t>::min());
}

TEST(LinearTableTest, DrivenValueSaturatesOnAdditionOverflow)
{
    wss::detail::linear_table table(make_linear_metadata(std::numeric_limits<std::int64_t>::max(), 1, 0));

    table.drive_to(1);

    EXPECT_EQ(table.driven_value(), std::numeric_limits<std::int64_t>::max());
}

TEST(LinearTableTest, DrivenValueSaturatesOnNegativeAdditionOverflow)
{
    wss::detail::linear_table table(make_linear_metadata(std::numeric_limits<std::int64_t>::min(), -1, 0));

    table.drive_to(1);

    EXPECT_EQ(table.driven_value(), std::numeric_limits<std::int64_t>::min());
}

TEST(LinearTableTest, ValueAtSaturatesWhenIndexDeltaOverflowsPositive)
{
    wss::detail::linear_table table(make_linear_metadata(0, 1, std::numeric_limits<std::int64_t>::min()));

    EXPECT_EQ(table.value_at(std::numeric_limits<std::int64_t>::max()), std::numeric_limits<std::int64_t>::max());
}

TEST(LinearTableTest, ValueAtSaturatesWhenIndexDeltaOverflowsNegative)
{
    wss::detail::linear_table table(make_linear_metadata(0, 1, std::numeric_limits<std::int64_t>::max()));

    EXPECT_EQ(table.value_at(std::numeric_limits<std::int64_t>::min()), std::numeric_limits<std::int64_t>::min());
}

TEST(LinearTableTest, ValueAtHandlesNegativeStoredIndexWithoutOverflow)
{
    wss::detail::linear_table table(make_linear_metadata(10, 2, -5));

    EXPECT_EQ(table.value_at(0), 20);
}

TEST(LinearTableTest, ValueAtSaturatesForMinTimesNegativeOneSpecialCase)
{
    wss::detail::linear_table table(make_linear_metadata(0, std::numeric_limits<std::int64_t>::min(), 0));

    EXPECT_EQ(table.value_at(-1), std::numeric_limits<std::int64_t>::max());
}

TEST(LinearTableTest, ValueAtSaturatesWhenPositiveDeltaMultipliesLargeNegativeOffset)
{
    wss::detail::linear_table table(make_linear_metadata(0, 2, 0));

    EXPECT_EQ(table.value_at(std::numeric_limits<std::int64_t>::min()), std::numeric_limits<std::int64_t>::min());
}

TEST(LinearTableTest, ValueAtHandlesNegativeDeltaAndNegativeOffset)
{
    wss::detail::linear_table table(make_linear_metadata(0, -2, 0));

    EXPECT_EQ(table.value_at(-3), 6);
}

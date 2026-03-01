#include <cstdint>
#include <limits>

#include <ws-streaming/metadata.hpp>
#include <ws-streaming/rule_types.hpp>
#include <ws-streaming/detail/linear_table.hpp>
#include <ws-streaming/detail/streaming_protocol.hpp>

#include <boost/endian/conversion.hpp>

namespace
{
    constexpr std::int64_t k_int64_min = std::numeric_limits<std::int64_t>::min();
    constexpr std::int64_t k_int64_max = std::numeric_limits<std::int64_t>::max();

    std::int64_t saturating_add(std::int64_t lhs, std::int64_t rhs) noexcept
    {
        if (rhs > 0 && lhs > k_int64_max - rhs)
            return k_int64_max;

        if (rhs < 0 && lhs < k_int64_min - rhs)
            return k_int64_min;

        return lhs + rhs;
    }

    std::int64_t saturating_sub(std::int64_t lhs, std::int64_t rhs) noexcept
    {
        if (rhs > 0 && lhs < k_int64_min + rhs)
            return k_int64_min;

        if (rhs < 0 && lhs > k_int64_max + rhs)
            return k_int64_max;

        return lhs - rhs;
    }

    std::int64_t saturating_mul(std::int64_t lhs, std::int64_t rhs) noexcept
    {
        if (lhs == 0 || rhs == 0)
            return 0;

        if ((lhs == k_int64_min && rhs == -1) || (rhs == k_int64_min && lhs == -1))
            return k_int64_max;

        if (lhs > 0)
        {
            if (rhs > 0)
            {
                if (lhs > k_int64_max / rhs)
                    return k_int64_max;
            }
            else if (rhs < k_int64_min / lhs)
            {
                return k_int64_min;
            }
        }
        else if (rhs > 0)
        {
            if (lhs < k_int64_min / rhs)
                return k_int64_min;
        }
        else if (lhs < k_int64_max / rhs)
        {
            return k_int64_max;
        }

        return lhs * rhs;
    }
}

wss::detail::linear_table::linear_table(const metadata& metadata)
{
    update(metadata);
}

void wss::detail::linear_table::update(
    const metadata& metadata)
{
    auto [start, delta] = metadata.linear_start_delta();

    _value = start.value_or(_value);
    _delta = delta.value_or(_delta);

    if (auto new_index = metadata.value_index(); new_index.has_value())
    {
        _index = new_index.value();
        _driven_index = _index;
    }
}

void wss::detail::linear_table::update(
    const streaming_protocol::linear_payload& payload)
{
    _driven_index = _index = boost::endian::little_to_native(payload.sample_index);
    _value = boost::endian::little_to_native(payload.value);
}

std::int64_t wss::detail::linear_table::driven_value() const noexcept
{
    const auto offset = saturating_sub(_driven_index, _index);
    return saturating_add(_value, saturating_mul(_delta, offset));
}

std::int64_t wss::detail::linear_table::value_at(std::int64_t index) const noexcept
{
    const auto offset = saturating_sub(index, _index);
    return saturating_add(_value, saturating_mul(_delta, offset));
}

void wss::detail::linear_table::set(std::int64_t index, std::int64_t value) noexcept
{
    _index = index;
    _value = value;
    _driven_index = index;
}

void wss::detail::linear_table::drive_to(std::int64_t index) noexcept
{
    _driven_index = index;
}

std::int64_t wss::detail::linear_table::driven_index() const noexcept
{
    return _driven_index;
}

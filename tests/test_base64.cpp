#include <array>
#include <cstdint>
#include <iterator>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <ws-streaming/detail/base64.hpp>

using namespace testing;
using namespace wss::detail;

namespace
{
    class ThrowingInputIterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = const char*;
        using reference = const char&;

        ThrowingInputIterator(
            const char* ptr,
            bool throw_on_deref,
            bool throw_on_increment,
            bool throw_on_copy = false)
            : ptr_(ptr),
              throw_on_deref_(throw_on_deref),
              throw_on_increment_(throw_on_increment),
              throw_on_copy_(throw_on_copy)
        {
        }

        ThrowingInputIterator(const ThrowingInputIterator& other)
            : ptr_(other.ptr_),
              throw_on_deref_(other.throw_on_deref_),
              throw_on_increment_(other.throw_on_increment_),
              throw_on_copy_(other.throw_on_copy_)
        {
            if (throw_on_copy_)
            {
                throw std::runtime_error("copy failed");
            }
        }

        reference operator*() const
        {
            if (throw_on_deref_)
            {
                throw std::runtime_error("dereference failed");
            }
            return *ptr_;
        }

        ThrowingInputIterator& operator++()
        {
            if (throw_on_increment_)
            {
                throw std::runtime_error("increment failed");
            }
            ++ptr_;
            return *this;
        }

        ThrowingInputIterator operator++(int)
        {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        friend bool operator==(const ThrowingInputIterator& lhs, const ThrowingInputIterator& rhs)
        {
            return lhs.ptr_ == rhs.ptr_;
        }

        friend bool operator!=(const ThrowingInputIterator& lhs, const ThrowingInputIterator& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        const char* ptr_;
        bool throw_on_deref_;
        bool throw_on_increment_;
        bool throw_on_copy_;
    };
}

TEST(Base64, CharStrings)
{
    EXPECT_EQ(base64(std::string("")), "");
    EXPECT_EQ(base64(std::string("f")), "Zg==");
    EXPECT_EQ(base64(std::string("fo")), "Zm8=");
    EXPECT_EQ(base64(std::string("foo")), "Zm9v");
    EXPECT_EQ(base64(std::string("foob")), "Zm9vYg==");
    EXPECT_EQ(base64(std::string("fooba")), "Zm9vYmE=");
    EXPECT_EQ(base64(std::string("foobar")), "Zm9vYmFy");
}

TEST(Base64, CharLiterals)
{
    // Note: Here the NULL terminator is included!
    EXPECT_EQ(base64(""), "AA==");
    EXPECT_EQ(base64("f"), "ZgA=");
    EXPECT_EQ(base64("fo"), "Zm8A");
    EXPECT_EQ(base64("foo"), "Zm9vAA==");
    EXPECT_EQ(base64("foob"), "Zm9vYgA=");
    EXPECT_EQ(base64("fooba"), "Zm9vYmEA");
    EXPECT_EQ(base64("foobar"), "Zm9vYmFyAA==");
}

TEST(Base64, CharArrays)
{
    EXPECT_EQ(base64(std::array<char, 0>{ }), "");
    EXPECT_EQ(base64(std::array<char, 1>{ 'f', }), "Zg==");
    EXPECT_EQ(base64(std::array<char, 2>{ 'f', 'o' }), "Zm8=");
    EXPECT_EQ(base64(std::array<char, 3>{ 'f', 'o', 'o' }), "Zm9v");
    EXPECT_EQ(base64(std::array<char, 4>{ 'f', 'o', 'o', 'b' }), "Zm9vYg==");
    EXPECT_EQ(base64(std::array<char, 5>{ 'f', 'o', 'o', 'b', 'a' }), "Zm9vYmE=");
    EXPECT_EQ(base64(std::array<char, 6>{ 'f', 'o', 'o', 'b', 'a', 'r' }), "Zm9vYmFy");
}

TEST(Base64, CharVectors)
{
    EXPECT_EQ(base64(std::vector<char>{ }), "");
    EXPECT_EQ(base64(std::vector<char>{ 'f', }), "Zg==");
    EXPECT_EQ(base64(std::vector<char>{ 'f', 'o' }), "Zm8=");
    EXPECT_EQ(base64(std::vector<char>{ 'f', 'o', 'o' }), "Zm9v");
    EXPECT_EQ(base64(std::vector<char>{ 'f', 'o', 'o', 'b' }), "Zm9vYg==");
    EXPECT_EQ(base64(std::vector<char>{ 'f', 'o', 'o', 'b', 'a' }), "Zm9vYmE=");
    EXPECT_EQ(base64(std::vector<char>{ 'f', 'o', 'o', 'b', 'a', 'r' }), "Zm9vYmFy");
}

TEST(Base64, ByteArrays)
{
    EXPECT_EQ(base64(std::array<std::uint8_t, 0>{ }), "");
    EXPECT_EQ(base64(std::array<std::uint8_t, 1>{ 1 }), "AQ==");
    EXPECT_EQ(base64(std::array<std::uint8_t, 2>{ 1, 2 }), "AQI=");
    EXPECT_EQ(base64(std::array<std::uint8_t, 3>{ 1, 2, 3 }), "AQID");
}

TEST(Base64, ByteVectors)
{
    EXPECT_EQ(base64(std::vector<std::uint8_t>{ }), "");
    EXPECT_EQ(base64(std::vector<std::uint8_t>{ 1 }), "AQ==");
    EXPECT_EQ(base64(std::vector<std::uint8_t>{ 1, 2 }), "AQI=");
    EXPECT_EQ(base64(std::vector<std::uint8_t>{ 1, 2, 3 }), "AQID");
}

TEST(Base64, NonRandomAccessIterators)
{
    const std::list<char> chars{ 'f', 'o', 'o' };
    EXPECT_EQ(base64(chars), "Zm9v");
    EXPECT_EQ(base64(chars.cbegin(), chars.cend()), "Zm9v");
}

TEST(Base64, HighBitCharBytes)
{
    const std::vector<char> high_bit_chars{
        static_cast<char>(0x80),
        static_cast<char>(0xFF),
    };
    const std::vector<std::uint8_t> bytes{ 0x80, 0xFF };

    EXPECT_EQ(base64(std::vector<char>{ static_cast<char>(0xFF) }), "/w==");
    EXPECT_EQ(base64(high_bit_chars), base64(bytes));
}

TEST(Base64, PropagatesIteratorExceptions)
{
    const std::array<char, 2> bytes{ 'f', 'o' };

    EXPECT_THROW(
        base64(
            ThrowingInputIterator(bytes.data(), true, false),
            ThrowingInputIterator(bytes.data() + bytes.size(), false, false)),
        std::runtime_error);

    EXPECT_THROW(
        base64(
            ThrowingInputIterator(bytes.data(), false, true),
            ThrowingInputIterator(bytes.data() + bytes.size(), false, false)),
        std::runtime_error);

    EXPECT_THROW(
        base64(
            ThrowingInputIterator(bytes.data(), false, false, true),
            ThrowingInputIterator(bytes.data() + bytes.size(), false, false)),
        std::runtime_error);

    EXPECT_THROW(
        base64(
            ThrowingInputIterator(bytes.data(), false, false),
            ThrowingInputIterator(bytes.data() + bytes.size(), false, false, true)),
        std::runtime_error);
}

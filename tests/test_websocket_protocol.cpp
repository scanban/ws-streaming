#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <ws-streaming/detail/websocket_protocol.hpp>

using namespace testing;
using namespace wss::detail;

TEST(WebSocketProtocol, ConstantsMatchProtocol)
{
    EXPECT_EQ(websocket_protocol::MAX_HEADER_SIZE, 10U);
    EXPECT_STREQ(websocket_protocol::MAGIC_KEY, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    EXPECT_EQ(websocket_protocol::flags::FIN, 0x80U);

    EXPECT_EQ(websocket_protocol::opcodes::TEXT, 1U);
    EXPECT_EQ(websocket_protocol::opcodes::BINARY, 2U);
    EXPECT_EQ(websocket_protocol::opcodes::CLOSE, 8U);
    EXPECT_EQ(websocket_protocol::opcodes::PING, 9U);
    EXPECT_EQ(websocket_protocol::opcodes::PONG, 10U);
}

TEST(WebSocketProtocol, GenerateHeaderReturnsZeroForNullDestination)
{
    EXPECT_EQ(websocket_protocol::generate_header(
        nullptr,
        websocket_protocol::opcodes::TEXT,
        websocket_protocol::flags::FIN,
        5),
        0U);
}

TEST(WebSocketProtocol, GenerateHeaderUsesInlinePayloadLength)
{
    std::array<std::uint8_t, websocket_protocol::MAX_HEADER_SIZE> header { };

    auto size = websocket_protocol::generate_header(
        header.data(),
        websocket_protocol::opcodes::TEXT,
        websocket_protocol::flags::FIN,
        125);

    ASSERT_EQ(size, 2U);
    EXPECT_EQ(header[0], static_cast<std::uint8_t>(websocket_protocol::flags::FIN |
        websocket_protocol::opcodes::TEXT));
    EXPECT_EQ(header[1], 125U);
}

TEST(WebSocketProtocol, GenerateHeaderUses16BitExtendedPayloadLength)
{
    std::array<std::uint8_t, websocket_protocol::MAX_HEADER_SIZE> header { };

    auto size = websocket_protocol::generate_header(
        header.data(),
        websocket_protocol::opcodes::BINARY,
        websocket_protocol::flags::FIN,
        65535);

    ASSERT_EQ(size, 4U);
    EXPECT_EQ(header[0], static_cast<std::uint8_t>(websocket_protocol::flags::FIN |
        websocket_protocol::opcodes::BINARY));
    EXPECT_EQ(header[1], 126U);
    EXPECT_EQ(header[2], 0xFFU);
    EXPECT_EQ(header[3], 0xFFU);
}

TEST(WebSocketProtocol, GenerateHeaderUses16BitExtendedPayloadLengthAtBoundary)
{
    std::array<std::uint8_t, websocket_protocol::MAX_HEADER_SIZE> header { };

    auto size = websocket_protocol::generate_header(
        header.data(),
        websocket_protocol::opcodes::BINARY,
        websocket_protocol::flags::FIN,
        126);

    ASSERT_EQ(size, 4U);
    EXPECT_EQ(header[1], 126U);
    EXPECT_EQ(header[2], 0x00U);
    EXPECT_EQ(header[3], 0x7EU);
}

TEST(WebSocketProtocol, GenerateHeaderUses64BitExtendedPayloadLength)
{
    std::array<std::uint8_t, websocket_protocol::MAX_HEADER_SIZE> header { };
    constexpr std::size_t payload = 65536;

    auto size = websocket_protocol::generate_header(
        header.data(),
        websocket_protocol::opcodes::BINARY,
        websocket_protocol::flags::FIN,
        payload);

    ASSERT_EQ(size, 10U);
    EXPECT_EQ(header[1], 127U);
    EXPECT_EQ(header[2], 0U);
    EXPECT_EQ(header[3], 0U);
    EXPECT_EQ(header[4], 0U);
    EXPECT_EQ(header[5], 0U);
    EXPECT_EQ(header[6], 0U);
    EXPECT_EQ(header[7], 1U);
    EXPECT_EQ(header[8], 0U);
    EXPECT_EQ(header[9], 0U);
}

TEST(WebSocketProtocol, DecodeHeaderReturnsEmptyForNullData)
{
    const auto zero_size = websocket_protocol::decode_header(nullptr, 0);
    const auto nonzero_size = websocket_protocol::decode_header(nullptr, 2);

    EXPECT_EQ(zero_size.header_size, 0U);
    EXPECT_EQ(nonzero_size.header_size, 0U);
}

TEST(WebSocketProtocol, DecodeHeaderReturnsEmptyForTooShortInput)
{
    const std::array<std::uint8_t, 1> frame { 0x81 };
    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 0U);
}

TEST(WebSocketProtocol, DecodeHeaderDecodesUnmaskedSmallPayload)
{
    const std::array<std::uint8_t, 4> frame {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::TEXT),
        2,
        0xAB,
        0xCD,
    };

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 2U);
    EXPECT_EQ(decoded.flags, websocket_protocol::flags::FIN);
    EXPECT_EQ(decoded.opcode, websocket_protocol::opcodes::TEXT);
    EXPECT_FALSE(decoded.is_masked);
    EXPECT_EQ(decoded.payload_size, 2U);
}

TEST(WebSocketProtocol, DecodeHeaderDecodesMaskedSmallPayload)
{
    const std::array<std::uint8_t, 7> frame {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::PING),
        0x80 | 1,
        0x12,
        0x34,
        0x56,
        0x78,
        0x9A,
    };

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 6U);
    EXPECT_EQ(decoded.flags, websocket_protocol::flags::FIN);
    EXPECT_EQ(decoded.opcode, websocket_protocol::opcodes::PING);
    EXPECT_TRUE(decoded.is_masked);
    EXPECT_EQ(decoded.payload_size, 1U);
    EXPECT_EQ(decoded.masking_key, (std::array<std::uint8_t, 4>{ 0x12, 0x34, 0x56, 0x78 }));
}

TEST(WebSocketProtocol, DecodeHeaderDecodes16BitExtendedPayloadLength)
{
    const std::array<std::uint8_t, 4> header_only {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::BINARY),
        126,
        0x00,
        0x7E,
    };
    std::vector<std::uint8_t> frame(header_only.begin(), header_only.end());
    frame.resize(header_only.size() + 126, 0x00);

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 4U);
    EXPECT_EQ(decoded.payload_size, 126U);
    EXPECT_FALSE(decoded.is_masked);
}

TEST(WebSocketProtocol, DecodeHeaderDecodesMasked16BitExtendedPayloadLength)
{
    const std::array<std::uint8_t, 8> header_only {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::BINARY),
        0x80 | 126,
        0x00,
        0x7E,
        0x01,
        0x02,
        0x03,
        0x04,
    };
    std::vector<std::uint8_t> frame(header_only.begin(), header_only.end());
    frame.resize(header_only.size() + 126, 0x55);

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 8U);
    EXPECT_EQ(decoded.payload_size, 126U);
    EXPECT_TRUE(decoded.is_masked);
    EXPECT_EQ(decoded.masking_key, (std::array<std::uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 }));
}

TEST(WebSocketProtocol, DecodeHeaderDecodes64BitExtendedPayloadLength)
{
    const std::array<std::uint8_t, 10> frame {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::BINARY),
        127,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    };

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 10U);
    EXPECT_EQ(decoded.payload_size, 0U);
    EXPECT_FALSE(decoded.is_masked);
}

TEST(WebSocketProtocol, DecodeHeaderDecodesMasked64BitExtendedPayloadLength)
{
    const std::array<std::uint8_t, 14> header_only {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::BINARY),
        0x80 | 127,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x01,
        0xAA,
        0xBB,
        0xCC,
        0xDD,
    };
    std::vector<std::uint8_t> frame(header_only.begin(), header_only.end());
    frame.push_back(0x42);

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 14U);
    EXPECT_EQ(decoded.payload_size, 1U);
    EXPECT_TRUE(decoded.is_masked);
    EXPECT_EQ(decoded.masking_key, (std::array<std::uint8_t, 4>{ 0xAA, 0xBB, 0xCC, 0xDD }));
}

TEST(WebSocketProtocol, DecodeHeaderReturnsEmptyWhen16BitLengthIsTruncated)
{
    const std::array<std::uint8_t, 3> frame {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::BINARY),
        126,
        0x00,
    };

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 0U);
}

TEST(WebSocketProtocol, DecodeHeaderReturnsEmptyWhen64BitLengthIsTruncated)
{
    const std::array<std::uint8_t, 9> frame {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::BINARY),
        127,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    };

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 0U);
}

TEST(WebSocketProtocol, DecodeHeaderReturnsEmptyWhenMaskingKeyIsTruncated)
{
    const std::array<std::uint8_t, 4> frame {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::TEXT),
        0x80 | 1,
        0xAA,
        0xBB,
    };

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 0U);
}

TEST(WebSocketProtocol, DecodeHeaderReturnsEmptyWhenMaskedPayloadIsTruncatedAfterMaskKey)
{
    const std::array<std::uint8_t, 7> frame {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::TEXT),
        0x80 | 2,
        0xDE,
        0xAD,
        0xBE,
        0xEF,
        0x33,
    };

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 0U);
    EXPECT_EQ(decoded.payload_size, 2U);
    EXPECT_TRUE(decoded.is_masked);
}

TEST(WebSocketProtocol, DecodeHeaderReturnsZeroHeaderSizeWhenPayloadIsTruncated)
{
    const std::array<std::uint8_t, 3> frame {
        static_cast<std::uint8_t>(websocket_protocol::flags::FIN | websocket_protocol::opcodes::TEXT),
        2,
        0x42,
    };

    const auto decoded = websocket_protocol::decode_header(frame.data(), frame.size());

    EXPECT_EQ(decoded.header_size, 0U);
    EXPECT_EQ(decoded.payload_size, 2U);
}

TEST(WebSocketProtocol, GetResponseKeyMatchesRfcSample)
{
    const std::string response_key =
        websocket_protocol::get_response_key("dGhlIHNhbXBsZSBub25jZQ==");

    EXPECT_EQ(response_key, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
}

TEST(WebSocketProtocol, GetResponseKeyIsDeterministic)
{
    const std::string key = "x3JJHMbDL1EzLkh9GBhXDw==";
    const auto first = websocket_protocol::get_response_key(key);
    const auto second = websocket_protocol::get_response_key(key);
    const auto empty = websocket_protocol::get_response_key("");

    EXPECT_EQ(first, second);
    EXPECT_FALSE(empty.empty());
}

#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include <ws-streaming/detail/streaming_protocol.hpp>

namespace
{
    std::vector<std::uint8_t> with_payload(const std::uint8_t *header, std::size_t header_size, std::size_t payload_size)
    {
        std::vector<std::uint8_t> frame(header, header + header_size);
        frame.resize(header_size + payload_size, 0U);
        return frame;
    }
}

using namespace wss::detail;

TEST(StreamingProtocol, GenerateHeaderThrowsForNullDestination)
{
    EXPECT_THROW(
        streaming_protocol::generate_header(nullptr, 1U, streaming_protocol::packet_type::DATA, 1U),
        std::invalid_argument);
}

TEST(StreamingProtocol, GenerateHeaderEncodesShortPayloadForWebSocketMode)
{
    std::array<std::uint8_t, streaming_protocol::MAX_HEADER_SIZE> header { };

    const auto header_size = streaming_protocol::generate_header(
        header.data(),
        0x54321U,
        streaming_protocol::packet_type::DATA,
        42U);

    ASSERT_EQ(header_size, sizeof(std::uint32_t));
    const auto frame = with_payload(header.data(), header_size, 42U);
    const auto decoded = streaming_protocol::decode_header(frame.data(), frame.size(), false);

    EXPECT_EQ(decoded.header_size, sizeof(std::uint32_t));
    EXPECT_EQ(decoded.signo, 0x54321U);
    EXPECT_EQ(decoded.type, streaming_protocol::packet_type::DATA);
    EXPECT_EQ(decoded.payload_size, 42U);
}

TEST(StreamingProtocol, GenerateHeaderEncodesExtendedPayloadForWebSocketMode)
{
    std::array<std::uint8_t, streaming_protocol::MAX_HEADER_SIZE> header { };

    const auto header_size = streaming_protocol::generate_header(
        header.data(),
        0xABCDU,
        streaming_protocol::packet_type::METADATA,
        256U);

    ASSERT_EQ(header_size, 2 * sizeof(std::uint32_t));
    const auto frame = with_payload(header.data(), header_size, 256U);
    const auto decoded = streaming_protocol::decode_header(frame.data(), frame.size(), false);

    EXPECT_EQ(decoded.header_size, 2 * sizeof(std::uint32_t));
    EXPECT_EQ(decoded.signo, 0xABCDU);
    EXPECT_EQ(decoded.type, streaming_protocol::packet_type::METADATA);
    EXPECT_EQ(decoded.payload_size, 256U);
}

TEST(StreamingProtocol, GenerateHeaderSupportsUint32MaxPayload)
{
    std::array<std::uint8_t, streaming_protocol::MAX_HEADER_SIZE> header { };

    EXPECT_NO_THROW(streaming_protocol::generate_header(
        header.data(),
        1U,
        streaming_protocol::packet_type::DATA,
        std::numeric_limits<std::uint32_t>::max()));
}

TEST(StreamingProtocol, GenerateHeaderThrowsForPayloadTooLarge)
{
    std::array<std::uint8_t, streaming_protocol::MAX_HEADER_SIZE> header { };
    const std::size_t too_large_payload = static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) + 1U;

    EXPECT_THROW(
        streaming_protocol::generate_header(
            header.data(),
            1U,
            streaming_protocol::packet_type::DATA,
            too_large_payload),
        std::runtime_error);
}

TEST(StreamingProtocol, DecodeHeaderReturnsDefaultForNullInput)
{
    const auto decoded_zero = streaming_protocol::decode_header(nullptr, 0U, false);
    const auto decoded_websocket = streaming_protocol::decode_header(nullptr, 4U, false);
    const auto decoded_tcp = streaming_protocol::decode_header(nullptr, 8U, true);

    EXPECT_EQ(decoded_zero.header_size, 0U);
    EXPECT_EQ(decoded_zero.signo, 0U);
    EXPECT_EQ(decoded_zero.type, 0U);
    EXPECT_EQ(decoded_zero.payload_size, 0U);

    EXPECT_EQ(decoded_websocket.header_size, 0U);
    EXPECT_EQ(decoded_tcp.header_size, 0U);
}

TEST(StreamingProtocol, DecodeHeaderReturnsDefaultForTruncatedBaseHeader)
{
    const std::array<std::uint8_t, 3> data { 0x11, 0x22, 0x33 };

    const auto decoded = streaming_protocol::decode_header(data.data(), data.size(), false);

    EXPECT_EQ(decoded.header_size, 0U);
    EXPECT_EQ(decoded.signo, 0U);
    EXPECT_EQ(decoded.type, 0U);
    EXPECT_EQ(decoded.payload_size, 0U);
}

TEST(StreamingProtocol, DecodeHeaderReturnsDefaultWhenExtendedLengthIsTruncated)
{
    const std::array<std::uint8_t, 4> data { 0x01, 0x23, 0x00, 0x10 };

    const auto decoded = streaming_protocol::decode_header(data.data(), data.size(), false);

    EXPECT_EQ(decoded.header_size, 0U);
}

TEST(StreamingProtocol, DecodeHeaderParsesTcpShortHeaderWithCompletePayload)
{
    std::vector<std::uint8_t> frame { 0x37, 0xBA, 0xBC, 0xDE };
    frame.resize(frame.size() + 123U, 0U);

    const auto decoded = streaming_protocol::decode_header(frame.data(), frame.size(), true);

    EXPECT_EQ(decoded.header_size, sizeof(std::uint32_t));
    EXPECT_EQ(decoded.type, 3U);
    EXPECT_EQ(decoded.signo, 0xABCDEU);
    EXPECT_EQ(decoded.payload_size, 123U);
}

TEST(StreamingProtocol, DecodeHeaderParsesTcpExtendedHeaderWithCompletePayload)
{
    std::vector<std::uint8_t> frame { 0x20, 0x01, 0x23, 0x45, 0x00, 0x00, 0x02, 0x00 };
    frame.resize(frame.size() + 512U, 0U);

    const auto decoded = streaming_protocol::decode_header(frame.data(), frame.size(), true);

    EXPECT_EQ(decoded.header_size, 2 * sizeof(std::uint32_t));
    EXPECT_EQ(decoded.type, 2U);
    EXPECT_EQ(decoded.signo, 0x12345U);
    EXPECT_EQ(decoded.payload_size, 512U);
}

TEST(StreamingProtocol, DecodeHeaderReturnsDefaultWhenPayloadIsIncomplete)
{
    std::array<std::uint8_t, streaming_protocol::MAX_HEADER_SIZE> header { };
    const auto header_size = streaming_protocol::generate_header(
        header.data(),
        0x22222U,
        streaming_protocol::packet_type::DATA,
        7U);

    const auto frame = with_payload(header.data(), header_size, 6U);
    const auto decoded = streaming_protocol::decode_header(frame.data(), frame.size(), false);

    EXPECT_EQ(decoded.header_size, 0U);
    EXPECT_EQ(decoded.payload_size, 7U);
}

TEST(StreamingProtocol, DecodeHeaderReturnsDefaultWhenExtendedPayloadIsIncomplete)
{
    std::vector<std::uint8_t> frame { 0x20, 0x01, 0x23, 0x45, 0x00, 0x00, 0x02, 0x00 };
    frame.resize(frame.size() + 511U, 0U);

    const auto decoded = streaming_protocol::decode_header(frame.data(), frame.size(), true);

    EXPECT_EQ(decoded.header_size, 0U);
    EXPECT_EQ(decoded.payload_size, 512U);
}

TEST(StreamingProtocol, DecodeHeaderParsesTcpExtendedLengthWithHighBitSetWithoutCompletingPacket)
{
    const std::array<std::uint8_t, 8> frame {
        0x10, 0x0A, 0xBC, 0xDE,
        0x80, 0x00, 0x00, 0x01
    };

    const auto decoded = streaming_protocol::decode_header(frame.data(), frame.size(), true);

    EXPECT_EQ(decoded.header_size, 0U);
    EXPECT_EQ(decoded.type, 1U);
    EXPECT_EQ(decoded.signo, 0xABCDEU);
    EXPECT_EQ(decoded.payload_size, 0x80000001U);
}

TEST(StreamingProtocol, DecodeHeaderParsesWebSocketExtendedLengthWithHighBitSetWithoutCompletingPacket)
{
    const std::array<std::uint8_t, 8> frame {
        0xCD, 0xAB, 0x01, 0x20,
        0x01, 0x00, 0x00, 0x80
    };

    const auto decoded = streaming_protocol::decode_header(frame.data(), frame.size(), false);

    EXPECT_EQ(decoded.header_size, 0U);
    EXPECT_EQ(decoded.type, 2U);
    EXPECT_EQ(decoded.signo, 0x1ABCDU);
    EXPECT_EQ(decoded.payload_size, 0x80000001U);
}

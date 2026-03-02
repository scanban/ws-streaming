#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#include <gtest/gtest.h>

#include <ws-streaming/detail/peer.hpp>
#include <ws-streaming/detail/streaming_protocol.hpp>
#include <ws-streaming/detail/websocket_protocol.hpp>

namespace
{
    std::vector<std::uint8_t> make_streaming_packet(
        unsigned signo,
        const std::vector<std::uint8_t>& payload)
    {
        std::array<std::uint8_t, wss::detail::streaming_protocol::MAX_HEADER_SIZE> streaming_header { };
        const auto streaming_header_size = wss::detail::streaming_protocol::generate_header(
            streaming_header.data(),
            signo,
            wss::detail::streaming_protocol::packet_type::DATA,
            payload.size());

        std::vector<std::uint8_t> packet;
        packet.insert(packet.end(), streaming_header.begin(), streaming_header.begin() + streaming_header_size);
        packet.insert(packet.end(), payload.begin(), payload.end());
        return packet;
    }

    std::vector<std::uint8_t> wrap_websocket_binary_frame(const std::vector<std::uint8_t>& payload)
    {
        std::array<std::uint8_t, wss::detail::websocket_protocol::MAX_HEADER_SIZE> websocket_header { };
        const auto websocket_header_size = wss::detail::websocket_protocol::generate_header(
            websocket_header.data(),
            wss::detail::websocket_protocol::opcodes::BINARY,
            wss::detail::websocket_protocol::flags::FIN,
            payload.size());

        std::vector<std::uint8_t> frame;
        frame.insert(frame.end(), websocket_header.begin(), websocket_header.begin() + websocket_header_size);
        frame.insert(frame.end(), payload.begin(), payload.end());
        return frame;
    }
}

TEST(PeerSecurityRegressions, ProcessBufferWsConsumesConcatenatedFramesWithoutBufferOverread)
{
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::socket socket{ioc};
    ASSERT_NO_THROW(socket.open(boost::asio::ip::tcp::v4()));

    auto peer = std::make_shared<wss::detail::peer>(std::move(socket), false, false, 4096U, 4096U);

    std::vector<std::pair<unsigned, std::vector<std::uint8_t>>> received_packets;
    peer->on_data_received.connect([&](unsigned signo, const std::uint8_t* data, std::size_t size)
    {
        received_packets.emplace_back(signo, std::vector<std::uint8_t>(data, data + size));
    });

    const auto packet_1 = make_streaming_packet(0x11111U, {0x10U, 0x20U, 0x30U});
    const auto packet_2 = make_streaming_packet(0x22222U, {0xAAU, 0xBBU, 0xCCU, 0xDDU});
    const auto frame_1 = wrap_websocket_binary_frame(packet_1);
    const auto frame_2 = wrap_websocket_binary_frame(packet_2);

    std::vector<std::uint8_t> concatenated;
    concatenated.reserve(frame_1.size() + frame_2.size());
    concatenated.insert(concatenated.end(), frame_1.begin(), frame_1.end());
    concatenated.insert(concatenated.end(), frame_2.begin(), frame_2.end());

    peer->run(concatenated.data(), concatenated.size());
    ioc.poll();

    ASSERT_EQ(received_packets.size(), 2U);
    EXPECT_EQ(received_packets[0].first, 0x11111U);
    EXPECT_EQ(received_packets[1].first, 0x22222U);
    EXPECT_EQ(received_packets[0].second, (std::vector<std::uint8_t>{0x10U, 0x20U, 0x30U}));
    EXPECT_EQ(received_packets[1].second, (std::vector<std::uint8_t>{0xAAU, 0xBBU, 0xCCU, 0xDDU}));
}

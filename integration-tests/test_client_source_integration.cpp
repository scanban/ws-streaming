#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <gtest/gtest.h>

#include <ws-streaming/ws-streaming.hpp>

namespace
{
    using namespace std::chrono_literals;

    struct test_state
    {
        std::mutex mutex;
        std::condition_variable condition;

        bool connect_done = false;
        bool connected = false;
        bool value_available = false;
        bool remote_value_subscribed = false;
        bool value_subscribed = false;
        bool data_received = false;

        std::size_t received_sample_count = 0;
        std::size_t received_size = 0;
        std::int64_t received_domain_value = 0;

        boost::system::error_code connect_ec;
    };

    template <typename Rep, typename Period, typename Predicate>
    bool wait_for(
        test_state& state,
        const std::chrono::duration<Rep, Period>& timeout,
        Predicate&& predicate)
    {
        std::unique_lock<std::mutex> lock(state.mutex);
        return state.condition.wait_for(lock, timeout, std::forward<Predicate>(predicate));
    }
}

TEST(ClientSourceIntegration, EndToEndDataFlowOverLoopbackTcp)
{
    static constexpr std::uint16_t port = 17414;
    static const std::string url = "ws://127.0.0.1:" + std::to_string(port);

    boost::asio::io_context ioc{1};
    wss::server server{ioc.get_executor()};
    server.add_listener(port);
    server.run();

    wss::local_signal time_signal{
        "/Time",
        wss::metadata_builder{"Time"}
            .data_type(wss::data_types::int64_t)
            .unit(wss::unit::seconds)
            .linear_rule(0, std::chrono::duration_cast<std::chrono::system_clock::duration>(1s).count() / 1000)
            .tick_resolution(
                std::chrono::system_clock::period::num,
                std::chrono::system_clock::period::den)
            .table("/Time")
            .build()};

    wss::local_signal value_signal{
        "/Value",
        wss::metadata_builder{"Value"}
            .data_type(wss::data_types::real64_t)
            .unit(wss::unit::volts)
            .range(-10, 10)
            .table(time_signal.id())
            .build()};

    test_state state;
    wss::connection_ptr client_connection;

    value_signal.on_subscribed.connect([&]()
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.value_subscribed = true;
        state.condition.notify_all();
    });

    server.on_available.connect([&](wss::connection_ptr connection, wss::remote_signal_ptr signal)
    {
        (void) connection;
        if (signal->id() != "/Value")
            return;

        signal->on_subscribed.connect([&]()
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.remote_value_subscribed = true;
            state.condition.notify_all();
        });

        signal->on_data_received.connect([&](std::int64_t domain_value, std::size_t sample_count, const void* data, std::size_t size)
        {
            (void) data;

            std::lock_guard<std::mutex> lock(state.mutex);
            state.data_received = true;
            state.received_domain_value = domain_value;
            state.received_sample_count = sample_count;
            state.received_size = size;
            state.condition.notify_all();
        });

        signal->subscribe();

        std::lock_guard<std::mutex> lock(state.mutex);
        state.value_available = true;
        state.condition.notify_all();
    });

    wss::client client{ioc.get_executor()};

    client.async_connect(url, [&](const boost::system::error_code& ec, wss::connection_ptr connection)
    {
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.connect_done = true;
            state.connected = !ec;
            state.connect_ec = ec;
            client_connection = connection;
        }

        if (!ec)
        {
            connection->add_local_signal(time_signal);
            connection->add_local_signal(value_signal);
        }

        state.condition.notify_all();
    });

    std::thread io_thread([&]()
    {
        ioc.run();
    });

    const bool connected = wait_for(state, 5s, [&]()
    {
        return state.connect_done;
    });
    EXPECT_TRUE(connected) << "Timed out waiting for connection result";

    {
        std::lock_guard<std::mutex> lock(state.mutex);
        EXPECT_TRUE(state.connect_done);
        EXPECT_TRUE(state.connected) << "Connection failed with error: " << state.connect_ec.message();
    }

    const bool subscribed = wait_for(state, 5s, [&]()
    {
        return state.value_subscribed;
    });
    EXPECT_TRUE(subscribed) << "Timed out waiting for /Value subscription";

    const bool remote_subscribed = wait_for(state, 5s, [&]()
    {
        return state.remote_value_subscribed;
    });
    EXPECT_TRUE(remote_subscribed) << "Timed out waiting for remote /Value subscribe metadata";

    std::vector<double> samples{0.1, 0.2, 0.3, 0.4};
    constexpr std::int64_t domain_value = 123456;
    bool received = false;

    for (int attempt = 0; attempt < 50; ++attempt)
    {
        value_signal.publish_data(
            domain_value,
            samples.size(),
            samples.data(),
            sizeof(decltype(samples)::value_type) * samples.size());

        received = wait_for(state, 100ms, [&]()
        {
            return state.data_received;
        });

        if (received)
            break;
    }

    EXPECT_TRUE(received) << "Timed out waiting for data on server sink side";

    {
        std::lock_guard<std::mutex> lock(state.mutex);
        EXPECT_TRUE(state.value_available);
        EXPECT_TRUE(state.value_subscribed);
        EXPECT_TRUE(state.remote_value_subscribed);
        EXPECT_TRUE(state.data_received);
        EXPECT_GT(state.received_sample_count, 0U);
        EXPECT_GT(state.received_size, 0U);
        EXPECT_EQ(state.received_domain_value, domain_value);
    }

    boost::asio::post(ioc, [&]()
    {
        if (client_connection)
            client_connection->close();
        server.close();
    });

    io_thread.join();
}

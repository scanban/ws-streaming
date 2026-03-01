#include <optional>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include <ws-streaming/detail/url.hpp>

using namespace testing;
using namespace wss::detail;

TEST(UrlTest, ParsesSchemeAndHostWithoutPortOrPath)
{
    url parsed("ws://localhost");

    EXPECT_EQ(parsed.scheme(), "ws");
    EXPECT_EQ(parsed.host_address(), "localhost");
    EXPECT_EQ(parsed.port_number(), std::nullopt);
    EXPECT_EQ(parsed.path(), "");
}

TEST(UrlTest, ParsesHostPortAndPath)
{
    url parsed("ws://localhost:7414/stream");

    ASSERT_TRUE(parsed.port_number().has_value());
    EXPECT_EQ(parsed.port_number().value(), 7414);
    EXPECT_EQ(parsed.path(), "/stream");
}

TEST(UrlTest, ParsesQueryAndFragmentAsPartOfPath)
{
    url parsed("ws://host/path?x=1#f");

    EXPECT_EQ(parsed.path(), "/path?x=1#f");
}

TEST(UrlTest, ParsesIpv6AndStripsBrackets)
{
    url without_port("ws://[::1]");
    url with_port("ws://[::1]:7414/path");

    EXPECT_EQ(without_port.host_address(), "::1");
    EXPECT_EQ(without_port.port_number(), std::nullopt);
    EXPECT_EQ(without_port.path(), "");

    EXPECT_EQ(with_port.host_address(), "::1");
    ASSERT_TRUE(with_port.port_number().has_value());
    EXPECT_EQ(with_port.port_number().value(), 7414);
    EXPECT_EQ(with_port.path(), "/path");
}

TEST(UrlTest, ParsesIpvFutureHost)
{
    url parsed("ws://[v1.a-b:!$&'()*+,;=]");

    EXPECT_EQ(parsed.host_address(), "v1.a-b:!$&'()*+,;=");
    EXPECT_EQ(parsed.port_number(), std::nullopt);
}

TEST(UrlTest, ParsesIpv4WithoutPortOrPath)
{
    url parsed("ws://127.0.0.1");

    EXPECT_EQ(parsed.host_address(), "127.0.0.1");
    EXPECT_EQ(parsed.port_number(), std::nullopt);
    EXPECT_EQ(parsed.path(), "");
}

TEST(UrlTest, ParsesIpv4WithPortAndPath)
{
    url parsed("ws://192.168.1.10:7414/stream?x=1#f");

    EXPECT_EQ(parsed.host_address(), "192.168.1.10");
    ASSERT_TRUE(parsed.port_number().has_value());
    EXPECT_EQ(parsed.port_number().value(), 7414);
    EXPECT_EQ(parsed.path(), "/stream?x=1#f");
}

TEST(UrlTest, AllowsNonWebsocketScheme)
{
    url parsed("tcp://host");

    EXPECT_EQ(parsed.scheme(), "tcp");
    EXPECT_EQ(parsed.host_address(), "host");
    EXPECT_EQ(parsed.port_number(), std::nullopt);
}

TEST(UrlTest, ThrowsInvalidArgumentForMalformedUrls)
{
    EXPECT_THROW(url(""), std::invalid_argument);
    EXPECT_THROW(url("ws:/host"), std::invalid_argument);
    EXPECT_THROW(url("ws://:7414"), std::invalid_argument);
    EXPECT_THROW(url("ws://[::1"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsGarbageAfterPort)
{
    EXPECT_THROW(url("ws://h:7414abc"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsNonNumericPortToken)
{
    EXPECT_THROW(url("ws://h:abc"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsOutOfRangePort)
{
    EXPECT_THROW(url("ws://h:70000"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsSpacesInHostAndPath)
{
    EXPECT_THROW(url("ws://local host"), std::invalid_argument);
    EXPECT_THROW(url("ws://host/has space"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsControlAndInvalidCharacters)
{
    EXPECT_THROW(url("ws://ho^st"), std::invalid_argument);
    EXPECT_THROW(url("ws://host/\tbad"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsIpv4OctetOutOfRange)
{
    EXPECT_THROW(url("ws://256.1.1.1"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsIpv4WrongOctetCount)
{
    EXPECT_THROW(url("ws://1.2.3"), std::invalid_argument);
    EXPECT_THROW(url("ws://1.2.3.4.5"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsIpv4EmptyOctet)
{
    EXPECT_THROW(url("ws://1..2.3"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsIpv4LeadingZeroOctet)
{
    EXPECT_THROW(url("ws://01.2.3.4"), std::invalid_argument);
    EXPECT_THROW(url("ws://1.02.3.4"), std::invalid_argument);
}

TEST(UrlTest, StrictRejectsMalformedIpLiterals)
{
    EXPECT_THROW(url("ws://[:::]"), std::invalid_argument);
    EXPECT_THROW(url("ws://[vXYZ]"), std::invalid_argument);
    EXPECT_THROW(url("ws://[%25eth0]"), std::invalid_argument);
    EXPECT_THROW(url("ws://[fe80::1%eth0]"), std::invalid_argument);
}

TEST(UrlTest, AcceptsIpv6ZoneIdentifierWhenPercentEncoded)
{
    url parsed("ws://[fe80::1%25eth0]/stream");

    EXPECT_EQ(parsed.host_address(), "fe80::1%25eth0");
    EXPECT_EQ(parsed.path(), "/stream");
}

TEST(UrlTest, AllowsMixedDottedHostAsRegName)
{
    url parsed("ws://1.2.a.4");

    EXPECT_EQ(parsed.host_address(), "1.2.a.4");
}

TEST(UrlTest, StrictRejectsMalformedPercentEncoding)
{
    EXPECT_THROW(url("ws://host/%"), std::invalid_argument);
    EXPECT_THROW(url("ws://host/%2"), std::invalid_argument);
    EXPECT_THROW(url("ws://host/%2G"), std::invalid_argument);
}

TEST(UrlTest, AcceptsValidPercentEncoding)
{
    url parsed("ws://host/a%20b");

    EXPECT_EQ(parsed.path(), "/a%20b");
}

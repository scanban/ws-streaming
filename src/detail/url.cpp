#include <charconv>
#include <cctype>
#include <limits>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

#include <ws-streaming/detail/url.hpp>

namespace
{
    bool is_ascii(unsigned char c)
    {
        return c <= 0x7f;
    }

    bool is_unreserved(unsigned char c)
    {
        return std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~';
    }

    bool is_sub_delim(unsigned char c)
    {
        switch (c)
        {
            case '!':
            case '$':
            case '&':
            case '\'':
            case '(':
            case ')':
            case '*':
            case '+':
            case ',':
            case ';':
            case '=':
                return true;

            default:
                return false;
        }
    }

    bool is_hex(unsigned char c)
    {
        return std::isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    bool validate_percent_encoded(std::string_view s, std::size_t& i)
    {
        if (i + 2 >= s.size())
            return false;

        if (!is_hex(static_cast<unsigned char>(s[i + 1])) ||
            !is_hex(static_cast<unsigned char>(s[i + 2])))
            return false;

        i += 2;
        return true;
    }

    bool validate_scheme(std::string_view scheme)
    {
        if (scheme.empty())
            return false;

        if (!std::isalpha(static_cast<unsigned char>(scheme[0])))
            return false;

        for (std::size_t i = 1; i < scheme.size(); ++i)
        {
            unsigned char c = static_cast<unsigned char>(scheme[i]);
            if (!(std::isalnum(c) || c == '+' || c == '-' || c == '.'))
                return false;
        }

        return true;
    }

    bool validate_reg_name(std::string_view host)
    {
        if (host.empty())
            return false;

        for (std::size_t i = 0; i < host.size(); ++i)
        {
            unsigned char c = static_cast<unsigned char>(host[i]);

            if (!is_ascii(c) || std::iscntrl(c) || std::isspace(c))
                return false;

            if (c == '%')
            {
                if (!validate_percent_encoded(host, i))
                    return false;
                continue;
            }

            if (!(is_unreserved(c) || is_sub_delim(c)))
                return false;
        }

        return true;
    }

    bool validate_ip_literal(std::string_view host)
    {
        if (host.empty())
            return false;

        for (std::size_t i = 0; i < host.size(); ++i)
        {
            unsigned char c = static_cast<unsigned char>(host[i]);

            if (!is_ascii(c) || std::iscntrl(c) || std::isspace(c))
                return false;

            if (c == '%')
            {
                if (!validate_percent_encoded(host, i))
                    return false;
                continue;
            }

            if (!(std::isalnum(c) || c == ':' || c == '.' || c == '-' || c == '_' || c == '~' ||
                  is_sub_delim(c)))
                return false;
        }

        return true;
    }

    bool validate_path_and_suffix(std::string_view path)
    {
        for (std::size_t i = 0; i < path.size(); ++i)
        {
            unsigned char c = static_cast<unsigned char>(path[i]);

            if (!is_ascii(c) || std::iscntrl(c) || std::isspace(c))
                return false;

            if (c == '%')
            {
                if (!validate_percent_encoded(path, i))
                    return false;
                continue;
            }

            if (c == '/' || c == '?' || c == '#')
                continue;

            if (!(is_unreserved(c) || is_sub_delim(c) || c == ':' || c == '@'))
                return false;
        }

        return true;
    }

    bool is_ipv4_candidate(std::string_view host)
    {
        if (host.empty())
            return false;

        bool has_dot = false;
        for (unsigned char c : host)
        {
            if (c == '.')
            {
                has_dot = true;
                continue;
            }

            if (!std::isdigit(c))
                return false;
        }

        return has_dot;
    }

    bool validate_ipv4_address(std::string_view host)
    {
        std::size_t start = 0;
        int octets = 0;

        while (start <= host.size())
        {
            std::size_t end = host.find('.', start);
            if (end == std::string_view::npos)
                end = host.size();

            std::string_view octet = host.substr(start, end - start);
            if (octet.empty() || octet.size() > 3)
                return false;

            if (octet.size() > 1 && octet[0] == '0')
                return false;

            unsigned int value = 0;
            const auto [ptr, ec] = std::from_chars(
                octet.data(),
                octet.data() + octet.size(),
                value);
            if (ec != std::errc() || ptr != octet.data() + octet.size() || value > 255)
                return false;

            ++octets;
            if (end == host.size())
                break;

            start = end + 1;
        }

        return octets == 4;
    }
}

wss::detail::url::url(std::string_view str)
{
    std::regex re{R"(^([^:]+)://([^\[\]:/]+|\[[^\[\]/]+\])(?::(\d+))?(/.*)?)"};
    std::smatch matches;
    std::string as_str{str};

    if (!std::regex_match(as_str, matches, re))
        throw std::invalid_argument("invalid URL");

    _scheme = matches[1];
    _host_address = matches[2];
    _path = matches[4];

    if (!validate_scheme(_scheme))
        throw std::invalid_argument("invalid URL");

    bool is_ip_literal = _host_address.length() > 2 &&
        _host_address[0] == '[' &&
        _host_address[_host_address.length() - 1] == ']';

    if (is_ip_literal)
    {
        auto host_inside_brackets = std::string_view(_host_address).substr(1, _host_address.length() - 2);
        if (!validate_ip_literal(host_inside_brackets))
            throw std::invalid_argument("invalid URL");
        _host_address = std::string(host_inside_brackets);
    }
    else if (is_ipv4_candidate(_host_address))
    {
        if (!validate_ipv4_address(_host_address))
            throw std::invalid_argument("invalid URL");
    }
    else if (!validate_reg_name(_host_address))
    {
        throw std::invalid_argument("invalid URL");
    }

    if (!_path.empty() && !validate_path_and_suffix(_path))
        throw std::invalid_argument("invalid URL");

    if (!matches[3].str().empty())
    {
        const std::string port_str = matches[3].str();
        unsigned int parsed_port = 0;
        const auto [ptr, ec] = std::from_chars(
            port_str.data(),
            port_str.data() + port_str.size(),
            parsed_port);

        if (ec != std::errc() || ptr != port_str.data() + port_str.size() ||
            parsed_port > std::numeric_limits<std::uint16_t>::max())
            throw std::invalid_argument("invalid URL");

        _port_number = static_cast<std::uint16_t>(parsed_port);
    }
}

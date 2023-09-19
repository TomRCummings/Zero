#pragma once

#include <functional>
#include <string>

struct GopherIdentifier {
    std::string domain;
    std::string port;
    std::string selector;

    bool operator==(const GopherIdentifier &other) const {
        return (domain == other.domain && port == other.port && selector == other.selector);
    };
};

template <>
struct std::hash<GopherIdentifier> {
    std::size_t operator()(const GopherIdentifier& gi) const {
        std::size_t domainH = std::hash<std::string>{}(gi.domain);
        std::size_t portH = std::hash<std::string>{}(gi.port);
        std::size_t selectorH = std::hash<std::string>{}(gi.selector);

        return ((domainH ^ (portH << 1)) >> 1) ^ (selectorH << 1);
    };
};
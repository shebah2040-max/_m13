#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace m130::access::crypto {

/// RFC 4648 base32 (A..Z,2..7) encode / decode without padding enforcement.
/// Used for storing TOTP shared secrets — the canonical transport format
/// for RFC 6238 / Google Authenticator compatibility.
class Base32
{
public:
    static std::string encode(const std::uint8_t* bytes, std::size_t n);
    static std::string encode(std::string_view bytes);

    /// Decode a base32 string. Accepts '=' padding and is case-insensitive;
    /// returns nullopt on invalid alphabet or truncated groups.
    static std::optional<std::vector<std::uint8_t>> decode(std::string_view s);
};

} // namespace m130::access::crypto

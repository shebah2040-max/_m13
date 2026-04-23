#include "Base32.h"

#include <cctype>

namespace m130::access::crypto {

namespace {
constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

int decodeChar(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '2' && c <= '7') return 26 + (c - '2');
    return -1;
}
} // namespace

std::string Base32::encode(const std::uint8_t* bytes, std::size_t n)
{
    std::string out;
    out.reserve(((n + 4) / 5) * 8);
    std::size_t i = 0;
    while (i < n) {
        std::uint64_t buf = 0;
        const std::size_t take = (n - i >= 5) ? 5 : (n - i);
        for (std::size_t k = 0; k < take; ++k) {
            buf = (buf << 8) | bytes[i + k];
        }
        buf <<= (5 - take) * 8;
        const std::size_t groups = (take * 8 + 4) / 5;
        for (std::size_t g = 0; g < 8; ++g) {
            if (g < groups) {
                const int idx = static_cast<int>((buf >> (35 - g * 5)) & 0x1f);
                out.push_back(kAlphabet[idx]);
            } else {
                out.push_back('=');
            }
        }
        i += take;
    }
    return out;
}

std::string Base32::encode(std::string_view bytes)
{
    return encode(reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size());
}

std::optional<std::vector<std::uint8_t>> Base32::decode(std::string_view s)
{
    std::vector<std::uint8_t> out;
    out.reserve((s.size() * 5) / 8);
    std::uint64_t buf = 0;
    int bits = 0;
    for (char c : s) {
        if (c == '=' || c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        const int v = decodeChar(c);
        if (v < 0) return std::nullopt;
        buf = (buf << 5) | static_cast<std::uint64_t>(v);
        bits += 5;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<std::uint8_t>((buf >> bits) & 0xff));
        }
    }
    return out;
}

} // namespace m130::access::crypto

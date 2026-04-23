#include "PasswordHasher.h"

#include "Pbkdf2.h"

#include <cstdint>
#include <sstream>
#include <vector>

namespace m130::access {

namespace {

// Minimal base64 (standard alphabet, '=' padding). Keeps the hasher self
// contained without pulling a general-purpose encoder in.
constexpr char kB64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string b64encode(const std::uint8_t* data, std::size_t n)
{
    std::string out;
    out.reserve(((n + 2) / 3) * 4);
    std::size_t i = 0;
    while (i < n) {
        const std::uint32_t b1 = data[i];
        const std::uint32_t b2 = (i + 1 < n) ? data[i + 1] : 0;
        const std::uint32_t b3 = (i + 2 < n) ? data[i + 2] : 0;
        const std::uint32_t v  = (b1 << 16) | (b2 << 8) | b3;
        out.push_back(kB64[(v >> 18) & 0x3f]);
        out.push_back(kB64[(v >> 12) & 0x3f]);
        out.push_back(i + 1 < n ? kB64[(v >> 6) & 0x3f] : '=');
        out.push_back(i + 2 < n ? kB64[ v       & 0x3f] : '=');
        i += 3;
    }
    return out;
}

int b64val(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return 26 + (c - 'a');
    if (c >= '0' && c <= '9') return 52 + (c - '0');
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

std::vector<std::uint8_t> b64decode(std::string_view s)
{
    std::vector<std::uint8_t> out;
    out.reserve((s.size() * 3) / 4);
    std::uint32_t buf = 0;
    int bits = 0;
    for (char c : s) {
        if (c == '=') break;
        const int v = b64val(c);
        if (v < 0) continue;
        buf = (buf << 6) | static_cast<std::uint32_t>(v);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<std::uint8_t>((buf >> bits) & 0xff));
        }
    }
    return out;
}

bool constantTimeEqual(const std::vector<std::uint8_t>& a,
                       const std::vector<std::uint8_t>& b) noexcept
{
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i) diff = static_cast<unsigned char>(diff | (a[i] ^ b[i]));
    return diff == 0;
}

} // namespace

std::string Pbkdf2Hasher::hash(std::string_view password, std::string_view salt) const
{
    constexpr std::size_t kDkLen = 32;
    const auto dk = crypto::Pbkdf2::derive(password, salt, _iterations, kDkLen);
    std::ostringstream os;
    os << "pbkdf2-sha256$i=" << _iterations << '$'
       << b64encode(reinterpret_cast<const std::uint8_t*>(salt.data()), salt.size())
       << '$' << b64encode(dk.data(), dk.size());
    return os.str();
}

bool Pbkdf2Hasher::verify(std::string_view password, std::string_view encoded) const
{
    if (encoded.size() < 30) return false;
    const auto p1 = encoded.find('$');
    if (p1 == std::string_view::npos) return false;
    const auto p2 = encoded.find('$', p1 + 1);
    if (p2 == std::string_view::npos) return false;
    const auto p3 = encoded.find('$', p2 + 1);
    if (p3 == std::string_view::npos) return false;

    const auto id     = encoded.substr(0, p1);
    const auto params = encoded.substr(p1 + 1, p2 - p1 - 1);
    const auto salt_b = encoded.substr(p2 + 1, p3 - p2 - 1);
    const auto hash_b = encoded.substr(p3 + 1);
    if (id != "pbkdf2-sha256") return false;
    if (params.substr(0, 2) != "i=") return false;

    std::uint32_t iters = 0;
    for (char c : params.substr(2)) {
        if (c < '0' || c > '9') return false;
        iters = iters * 10 + static_cast<std::uint32_t>(c - '0');
    }
    if (iters == 0) return false;

    const auto salt = b64decode(salt_b);
    const auto want = b64decode(hash_b);
    if (want.empty()) return false;

    const auto got = crypto::Pbkdf2::derive(
        password,
        std::string_view(reinterpret_cast<const char*>(salt.data()), salt.size()),
        iters,
        want.size());
    return constantTimeEqual(got, want);
}

std::shared_ptr<IPasswordHasher> defaultHasher()
{
    return std::make_shared<Pbkdf2Hasher>();
}

} // namespace m130::access

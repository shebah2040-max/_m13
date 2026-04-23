#include "Totp.h"

#include "logging/Hmac.h"

#include <cstring>

namespace m130::access::crypto {

namespace {
constexpr std::uint32_t kPow10[] = {
    1u, 10u, 100u, 1000u, 10000u, 100000u, 1000000u, 10000000u, 100000000u
};

std::string zeroPad(std::uint32_t v, int digits)
{
    std::string out(static_cast<std::size_t>(digits), '0');
    for (int i = digits - 1; i >= 0 && v; --i) {
        out[static_cast<std::size_t>(i)] = static_cast<char>('0' + (v % 10));
        v /= 10;
    }
    return out;
}

bool constantTimeEqual(std::string_view a, std::string_view b) noexcept
{
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        diff = static_cast<unsigned char>(
            diff | (static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i])));
    }
    return diff == 0;
}
} // namespace

std::string Totp::hotp(std::string_view secret, std::uint64_t counter, int digits)
{
    if (digits < 1 || digits > 8) digits = kDefaultDigits;

    std::uint8_t msg[8];
    for (int i = 7; i >= 0; --i) {
        msg[i] = static_cast<std::uint8_t>(counter & 0xff);
        counter >>= 8;
    }

    logging::crypto::HmacSha256 h;
    h.init(secret.data(), secret.size());
    h.update(msg, sizeof(msg));
    const auto digest = h.finalize();

    const int offset = digest[digest.size() - 1] & 0x0f;
    const std::uint32_t bin =
        (static_cast<std::uint32_t>(digest[offset]     & 0x7f) << 24) |
        (static_cast<std::uint32_t>(digest[offset + 1] & 0xff) << 16) |
        (static_cast<std::uint32_t>(digest[offset + 2] & 0xff) << 8)  |
         static_cast<std::uint32_t>(digest[offset + 3] & 0xff);

    return zeroPad(bin % kPow10[digits], digits);
}

std::string Totp::totp(std::string_view secret, std::uint64_t now_s,
                       std::uint64_t step_s, int digits)
{
    if (step_s == 0) step_s = kDefaultStepSeconds;
    return hotp(secret, now_s / step_s, digits);
}

bool Totp::verify(std::string_view secret, std::uint64_t now_s, std::string_view code,
                  std::uint64_t step_s, int digits, int lookback)
{
    if (step_s == 0) step_s = kDefaultStepSeconds;
    const std::uint64_t current = now_s / step_s;
    for (int off = -lookback; off <= lookback; ++off) {
        std::uint64_t counter = current;
        if (off < 0) {
            const auto d = static_cast<std::uint64_t>(-off);
            if (d > current) continue;
            counter -= d;
        } else {
            counter += static_cast<std::uint64_t>(off);
        }
        if (constantTimeEqual(hotp(secret, counter, digits), code)) return true;
    }
    return false;
}

} // namespace m130::access::crypto

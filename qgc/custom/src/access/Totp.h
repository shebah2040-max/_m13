#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace m130::access::crypto {

/// RFC 4226 HOTP and RFC 6238 TOTP over HMAC-SHA-256.
///
/// Why SHA-256: more conservative than SHA-1 (still the most common TOTP
/// variant for legacy Google Authenticator compatibility). Modern authenticator
/// apps (Authy, 1Password, current Google Authenticator, Aegis) accept the
/// `algorithm=SHA256` parameter in the `otpauth://` URI.
///
/// Counter encoding: 8-byte big-endian per RFC 4226 §5.1.
/// Dynamic truncation per RFC 4226 §5.3.
class Totp
{
public:
    static constexpr std::uint64_t kDefaultStepSeconds = 30;
    static constexpr int           kDefaultDigits      = 6;
    static constexpr int           kDefaultLookback    = 1; ///< ± windows accepted

    /// HOTP — returns a `digits`-long zero-padded decimal code. @p secret
    /// is the raw shared secret (NOT base32 — decode first).
    static std::string hotp(std::string_view secret, std::uint64_t counter,
                            int digits = kDefaultDigits);

    /// TOTP — convenience wrapper: floor(now_s / step) -> HOTP.
    static std::string totp(std::string_view secret, std::uint64_t now_s,
                            std::uint64_t step_s = kDefaultStepSeconds,
                            int digits = kDefaultDigits);

    /// Verify with ± @p lookback windows. Constant-time code comparison.
    static bool verify(std::string_view secret, std::uint64_t now_s,
                       std::string_view code,
                       std::uint64_t step_s = kDefaultStepSeconds,
                       int digits = kDefaultDigits,
                       int lookback = kDefaultLookback);
};

} // namespace m130::access::crypto

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace m130::access::crypto {

/// PBKDF2-HMAC-SHA-256 (RFC 8018 §5.2). Used by `Pbkdf2Hasher` in
/// `PasswordHasher.h` to derive password hashes.
///
/// Default iteration count (`kDefaultIterations`) follows the OWASP Password
/// Storage Cheat Sheet recommendation (≥ 600 000 for SHA-256, 2023).
class Pbkdf2
{
public:
    static constexpr std::uint32_t kDefaultIterations = 600000;

    /// Derive @p dklen bytes of key material from @p password + @p salt.
    static std::vector<std::uint8_t> derive(std::string_view password,
                                            std::string_view salt,
                                            std::uint32_t iterations,
                                            std::size_t dklen);
};

} // namespace m130::access::crypto

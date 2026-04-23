#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace m130::access::crypto {

/// Parameters for a single Argon2id invocation (RFC 9106 §3.1).
struct Argon2Params {
    std::uint32_t memory_kib   = 19456;  ///< m in KiB (min 8 * p)
    std::uint32_t iterations   = 2;      ///< t (min 1)
    std::uint32_t parallelism  = 1;      ///< p (1..255 in practice)
    std::uint32_t tag_length   = 32;     ///< τ in bytes (min 4)

    /// RFC 9106 constant — only version 0x13 is supported.
    static constexpr std::uint32_t kVersion = 0x13;
};

/// Reasons `argon2idHash` may fail (returned by-value instead of exception so
/// callers in the access chain can branch without try/catch).
enum class Argon2Error : std::uint8_t {
    Ok               = 0,
    BadMemory        = 1,  ///< m < 8 * p
    BadIterations    = 2,  ///< t < 1
    BadParallelism   = 3,  ///< p < 1
    BadTagLength     = 4,  ///< τ < 4
};

/// Compute the Argon2id tag of length `p.tag_length` using `password` and
/// `salt`. `secret` and `associated` are RFC 9106 K and X; pass empty views
/// when unused. Returns an empty vector on parameter error — callers should
/// validate with `validateParams()` before calling.
std::vector<std::uint8_t> argon2idHash(std::string_view password,
                                       std::string_view salt,
                                       std::string_view secret,
                                       std::string_view associated,
                                       const Argon2Params& p);

/// Convenience overload with no secret / associated data.
std::vector<std::uint8_t> argon2idHash(std::string_view password,
                                       std::string_view salt,
                                       const Argon2Params& p);

/// Validate `p` and return `Ok` if the parameters are usable.
Argon2Error validateParams(const Argon2Params& p) noexcept;

} // namespace m130::access::crypto

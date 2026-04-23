#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>

namespace m130::logging::crypto {

/// FIPS 180-4 SHA-256. Pure stdlib; no OpenSSL. Tested against NIST vectors
/// (empty, "abc", 1M 'a's) in Sha256Test.cc.
class Sha256
{
public:
    static constexpr std::size_t kDigestSize = 32;
    using Digest = std::array<std::uint8_t, kDigestSize>;

    Sha256() noexcept { reset(); }

    void reset() noexcept;
    void update(const void* data, std::size_t len) noexcept;
    void update(std::string_view sv) noexcept { update(sv.data(), sv.size()); }

    /// Finalise and return the digest. Calling reset() afterwards allows reuse.
    Digest finalize() noexcept;

    /// One-shot helper.
    static Digest hash(const void* data, std::size_t len) noexcept;
    static Digest hash(std::string_view sv) noexcept { return hash(sv.data(), sv.size()); }

    /// Lowercase hex (64 chars).
    static std::string toHex(const Digest& d);

private:
    void _transform(const std::uint8_t block[64]) noexcept;

    std::uint32_t _h[8]{};
    std::uint64_t _bits{0};
    std::uint8_t  _buf[64]{};
    std::size_t   _buf_len{0};
};

} // namespace m130::logging::crypto

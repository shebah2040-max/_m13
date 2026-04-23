#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace m130::access::crypto {

/// BLAKE2b hash function per RFC 7693.
///
/// Produces digests of any length 1..64 bytes. Supports keyed mode and a
/// customisable digest length specified at construction. This pure-C++
/// implementation exists so the M130 core can depend on Argon2id without
/// pulling libsodium into the build — the hash is used both as a standalone
/// MAC/digest primitive and as the inner mixing function for Argon2id.
class Blake2b
{
public:
    static constexpr std::size_t kMaxDigest = 64;
    static constexpr std::size_t kBlockSize = 128;
    static constexpr std::size_t kMaxKey    = 64;

    /// Unkeyed BLAKE2b with a @p digest_len byte output (1..64).
    explicit Blake2b(std::size_t digest_len = 64);

    /// Keyed BLAKE2b (RFC 7693 §2.9): key length 0..64.
    Blake2b(std::size_t digest_len, std::string_view key);

    void update(const std::uint8_t* data, std::size_t n);
    void update(std::string_view s);

    /// Finalise and return the digest. Do not call more than once per
    /// instance.
    std::vector<std::uint8_t> finish();

    /// One-shot helper: hashes @p data and returns an owned digest.
    static std::vector<std::uint8_t> oneShot(std::string_view data,
                                             std::size_t       digest_len = 64);

    /// One-shot keyed helper.
    static std::vector<std::uint8_t> oneShotKeyed(std::string_view data,
                                                  std::string_view key,
                                                  std::size_t      digest_len);

private:
    void _compress(bool last) noexcept;

    std::array<std::uint64_t, 8> _h{};
    std::array<std::uint8_t, kBlockSize> _buf{};
    std::size_t   _buf_len = 0;
    std::uint64_t _t0 = 0;
    std::uint64_t _t1 = 0;
    std::size_t   _digest_len = 64;
};

} // namespace m130::access::crypto

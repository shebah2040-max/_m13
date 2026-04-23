#pragma once

#include "Sha256.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace m130::logging::crypto {

/// RFC 2104 HMAC over SHA-256. Tested against RFC 4231 vectors in HmacSha256Test.cc.
class HmacSha256
{
public:
    static constexpr std::size_t kBlockSize = 64;
    static constexpr std::size_t kDigestSize = Sha256::kDigestSize;
    using Digest = Sha256::Digest;

    HmacSha256() noexcept = default;

    void init(const void* key, std::size_t key_len) noexcept;
    void update(const void* data, std::size_t len) noexcept;
    void update(std::string_view sv) noexcept { update(sv.data(), sv.size()); }
    Digest finalize() noexcept;

    static Digest mac(std::string_view key, std::string_view data) noexcept;
    static std::string macHex(std::string_view key, std::string_view data);

private:
    Sha256 _inner;
    std::uint8_t _k_opad[kBlockSize]{};
    bool _initialized{false};
};

} // namespace m130::logging::crypto

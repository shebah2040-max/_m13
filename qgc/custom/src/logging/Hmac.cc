#include "Hmac.h"

#include <cstring>

namespace m130::logging::crypto {

void HmacSha256::init(const void* key, std::size_t key_len) noexcept
{
    std::uint8_t k0[kBlockSize] = {};

    if (key_len > kBlockSize) {
        const auto d = Sha256::hash(key, key_len);
        std::memcpy(k0, d.data(), Sha256::kDigestSize);
    } else {
        std::memcpy(k0, key, key_len);
    }

    std::uint8_t k_ipad[kBlockSize];
    for (std::size_t i = 0; i < kBlockSize; ++i) {
        k_ipad[i]   = static_cast<std::uint8_t>(k0[i] ^ 0x36);
        _k_opad[i]  = static_cast<std::uint8_t>(k0[i] ^ 0x5c);
    }
    _inner.reset();
    _inner.update(k_ipad, kBlockSize);
    _initialized = true;
}

void HmacSha256::update(const void* data, std::size_t len) noexcept
{
    _inner.update(data, len);
}

HmacSha256::Digest HmacSha256::finalize() noexcept
{
    const auto inner = _inner.finalize();
    Sha256 outer;
    outer.update(_k_opad, kBlockSize);
    outer.update(inner.data(), inner.size());
    _initialized = false;
    return outer.finalize();
}

HmacSha256::Digest HmacSha256::mac(std::string_view key, std::string_view data) noexcept
{
    HmacSha256 h;
    h.init(key.data(), key.size());
    h.update(data.data(), data.size());
    return h.finalize();
}

std::string HmacSha256::macHex(std::string_view key, std::string_view data)
{
    return Sha256::toHex(mac(key, data));
}

} // namespace m130::logging::crypto

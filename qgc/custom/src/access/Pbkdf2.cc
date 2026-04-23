#include "Pbkdf2.h"

#include "logging/Hmac.h"
#include "logging/Sha256.h"

#include <cstring>

namespace m130::access::crypto {

std::vector<std::uint8_t> Pbkdf2::derive(std::string_view password,
                                         std::string_view salt,
                                         std::uint32_t iterations,
                                         std::size_t dklen)
{
    using Hmac = logging::crypto::HmacSha256;
    constexpr std::size_t kHLen = logging::crypto::Sha256::kDigestSize;

    std::vector<std::uint8_t> out(dklen);
    if (iterations == 0 || dklen == 0) return out;

    const std::size_t blocks = (dklen + kHLen - 1) / kHLen;
    std::vector<std::uint8_t> u(kHLen);
    std::vector<std::uint8_t> t(kHLen);
    std::uint8_t be_i[4];

    for (std::size_t i = 1; i <= blocks; ++i) {
        be_i[0] = static_cast<std::uint8_t>((i >> 24) & 0xff);
        be_i[1] = static_cast<std::uint8_t>((i >> 16) & 0xff);
        be_i[2] = static_cast<std::uint8_t>((i >>  8) & 0xff);
        be_i[3] = static_cast<std::uint8_t>( i        & 0xff);

        Hmac h;
        h.init(password.data(), password.size());
        h.update(salt.data(), salt.size());
        h.update(be_i, 4);
        const auto d = h.finalize();
        std::memcpy(u.data(), d.data(), kHLen);
        std::memcpy(t.data(), d.data(), kHLen);

        for (std::uint32_t r = 1; r < iterations; ++r) {
            Hmac inner;
            inner.init(password.data(), password.size());
            inner.update(u.data(), kHLen);
            const auto d2 = inner.finalize();
            for (std::size_t k = 0; k < kHLen; ++k) {
                u[k] = d2[k];
                t[k] = static_cast<std::uint8_t>(t[k] ^ u[k]);
            }
        }

        const std::size_t offset = (i - 1) * kHLen;
        const std::size_t take = (offset + kHLen <= dklen) ? kHLen : (dklen - offset);
        std::memcpy(out.data() + offset, t.data(), take);
    }
    return out;
}

} // namespace m130::access::crypto

#include "Sha256.h"

#include <cstring>
#include <sstream>
#include <iomanip>

namespace m130::logging::crypto {

namespace {
constexpr std::uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

inline std::uint32_t rotr(std::uint32_t x, unsigned n) noexcept
{
    return (x >> n) | (x << (32u - n));
}
} // namespace

void Sha256::reset() noexcept
{
    _h[0] = 0x6a09e667; _h[1] = 0xbb67ae85; _h[2] = 0x3c6ef372; _h[3] = 0xa54ff53a;
    _h[4] = 0x510e527f; _h[5] = 0x9b05688c; _h[6] = 0x1f83d9ab; _h[7] = 0x5be0cd19;
    _bits = 0;
    _buf_len = 0;
}

void Sha256::_transform(const std::uint8_t block[64]) noexcept
{
    std::uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<std::uint32_t>(block[i*4    ]) << 24) |
               (static_cast<std::uint32_t>(block[i*4 + 1]) << 16) |
               (static_cast<std::uint32_t>(block[i*4 + 2]) <<  8) |
               (static_cast<std::uint32_t>(block[i*4 + 3])      );
    }
    for (int i = 16; i < 64; ++i) {
        const std::uint32_t s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
        const std::uint32_t s1 = rotr(w[i- 2], 17) ^ rotr(w[i- 2], 19) ^ (w[i- 2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    std::uint32_t a = _h[0], b = _h[1], c = _h[2], d = _h[3];
    std::uint32_t e = _h[4], f = _h[5], g = _h[6], h = _h[7];

    for (int i = 0; i < 64; ++i) {
        const std::uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        const std::uint32_t ch = (e & f) ^ (~e & g);
        const std::uint32_t t1 = h + S1 + ch + K[i] + w[i];
        const std::uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        const std::uint32_t mj = (a & b) ^ (a & c) ^ (b & c);
        const std::uint32_t t2 = S0 + mj;
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    _h[0] += a; _h[1] += b; _h[2] += c; _h[3] += d;
    _h[4] += e; _h[5] += f; _h[6] += g; _h[7] += h;
}

void Sha256::update(const void* data, std::size_t len) noexcept
{
    const auto* p = static_cast<const std::uint8_t*>(data);
    _bits += static_cast<std::uint64_t>(len) * 8ULL;

    if (_buf_len > 0) {
        const std::size_t want = 64 - _buf_len;
        const std::size_t take = (len < want) ? len : want;
        std::memcpy(_buf + _buf_len, p, take);
        _buf_len += take;
        p += take;
        len -= take;
        if (_buf_len == 64) {
            _transform(_buf);
            _buf_len = 0;
        }
    }
    while (len >= 64) {
        _transform(p);
        p += 64;
        len -= 64;
    }
    if (len > 0) {
        std::memcpy(_buf, p, len);
        _buf_len = len;
    }
}

Sha256::Digest Sha256::finalize() noexcept
{
    const std::uint64_t bits = _bits;
    std::uint8_t pad = 0x80;
    update(&pad, 1);
    std::uint8_t zero = 0x00;
    while (_buf_len != 56) {
        update(&zero, 1);
    }
    std::uint8_t lenbe[8];
    for (int i = 0; i < 8; ++i) {
        lenbe[i] = static_cast<std::uint8_t>((bits >> (56 - i*8)) & 0xFF);
    }
    update(lenbe, 8);

    Digest d{};
    for (int i = 0; i < 8; ++i) {
        d[i*4    ] = static_cast<std::uint8_t>((_h[i] >> 24) & 0xFF);
        d[i*4 + 1] = static_cast<std::uint8_t>((_h[i] >> 16) & 0xFF);
        d[i*4 + 2] = static_cast<std::uint8_t>((_h[i] >>  8) & 0xFF);
        d[i*4 + 3] = static_cast<std::uint8_t>((_h[i]      ) & 0xFF);
    }
    return d;
}

Sha256::Digest Sha256::hash(const void* data, std::size_t len) noexcept
{
    Sha256 h;
    h.update(data, len);
    return h.finalize();
}

std::string Sha256::toHex(const Digest& d)
{
    std::ostringstream os;
    os << std::hex << std::setfill('0');
    for (auto b : d) {
        os << std::setw(2) << static_cast<unsigned>(b);
    }
    return os.str();
}

} // namespace m130::logging::crypto

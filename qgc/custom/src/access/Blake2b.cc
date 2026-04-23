#include "Blake2b.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace m130::access::crypto {

namespace {

constexpr std::array<std::uint64_t, 8> kIv = {
    0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
    0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
    0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
    0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL,
};

constexpr std::uint8_t kSigma[12][16] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15},
    {14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3},
    {11, 8,12, 0, 5, 2,15,13,10,14, 3, 6, 7, 1, 9, 4},
    { 7, 9, 3, 1,13,12,11,14, 2, 6, 5,10, 4, 0,15, 8},
    { 9, 0, 5, 7, 2, 4,10,15,14, 1,11,12, 6, 8, 3,13},
    { 2,12, 6,10, 0,11, 8, 3, 4,13, 7, 5,15,14, 1, 9},
    {12, 5, 1,15,14,13, 4,10, 0, 7, 6, 3, 9, 2, 8,11},
    {13,11, 7,14,12, 1, 3, 9, 5, 0,15, 4, 8, 6, 2,10},
    { 6,15,14, 9,11, 3, 0, 8,12, 2,13, 7, 1, 4,10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5,15,11, 9,14, 3,12,13, 0},
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15},
    {14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3},
};

inline std::uint64_t rotr64(std::uint64_t x, unsigned n) noexcept
{
    return (x >> n) | (x << (64 - n));
}

inline std::uint64_t loadLe64(const std::uint8_t* p) noexcept
{
    std::uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= static_cast<std::uint64_t>(p[i]) << (8 * i);
    return v;
}

inline void storeLe64(std::uint8_t* p, std::uint64_t v) noexcept
{
    for (int i = 0; i < 8; ++i) p[i] = static_cast<std::uint8_t>((v >> (8 * i)) & 0xff);
}

inline void mix(std::uint64_t& a, std::uint64_t& b, std::uint64_t& c, std::uint64_t& d,
                std::uint64_t x, std::uint64_t y) noexcept
{
    a = a + b + x; d = rotr64(d ^ a, 32);
    c = c + d;     b = rotr64(b ^ c, 24);
    a = a + b + y; d = rotr64(d ^ a, 16);
    c = c + d;     b = rotr64(b ^ c, 63);
}

} // namespace

Blake2b::Blake2b(std::size_t digest_len) : Blake2b(digest_len, {}) {}

Blake2b::Blake2b(std::size_t digest_len, std::string_view key)
{
    if (digest_len == 0 || digest_len > kMaxDigest) {
        throw std::invalid_argument("blake2b: digest_len must be 1..64");
    }
    if (key.size() > kMaxKey) {
        throw std::invalid_argument("blake2b: key must be 0..64 bytes");
    }
    _digest_len = digest_len;
    _h = kIv;
    // Parameter block per RFC 7693 §2.5 (only digest_len, key_len, fanout=1, depth=1).
    const std::uint64_t param0 =
          static_cast<std::uint64_t>(digest_len)
        | (static_cast<std::uint64_t>(key.size()) << 8)
        | (static_cast<std::uint64_t>(1) << 16)   // fanout
        | (static_cast<std::uint64_t>(1) << 24);  // depth
    _h[0] ^= param0;

    if (!key.empty()) {
        std::array<std::uint8_t, kBlockSize> block{};
        std::memcpy(block.data(), key.data(), key.size());
        update(block.data(), block.size());
    }
}

void Blake2b::update(std::string_view s)
{
    update(reinterpret_cast<const std::uint8_t*>(s.data()), s.size());
}

void Blake2b::update(const std::uint8_t* data, std::size_t n)
{
    while (n > 0) {
        if (_buf_len == kBlockSize) {
            _t0 += kBlockSize;
            if (_t0 < kBlockSize) ++_t1;
            _compress(false);
            _buf_len = 0;
        }
        const std::size_t take = std::min(n, kBlockSize - _buf_len);
        std::memcpy(_buf.data() + _buf_len, data, take);
        _buf_len += take;
        data     += take;
        n        -= take;
    }
}

std::vector<std::uint8_t> Blake2b::finish()
{
    _t0 += _buf_len;
    if (_t0 < _buf_len) ++_t1;
    // Zero-pad the remainder of the buffer.
    for (std::size_t i = _buf_len; i < kBlockSize; ++i) _buf[i] = 0;
    _compress(true);

    std::vector<std::uint8_t> out(_digest_len);
    std::uint8_t tmp[kMaxDigest]{};
    for (int i = 0; i < 8; ++i) storeLe64(&tmp[i * 8], _h[i]);
    std::memcpy(out.data(), tmp, _digest_len);
    return out;
}

void Blake2b::_compress(bool last) noexcept
{
    std::uint64_t v[16];
    std::uint64_t m[16];
    for (int i = 0; i < 16; ++i) m[i] = loadLe64(&_buf[i * 8]);
    for (int i = 0; i < 8;  ++i) v[i]      = _h[i];
    for (int i = 0; i < 8;  ++i) v[i + 8]  = kIv[i];
    v[12] ^= _t0;
    v[13] ^= _t1;
    if (last) v[14] = ~v[14];

    for (int r = 0; r < 12; ++r) {
        const auto& s = kSigma[r];
        mix(v[0], v[4], v[ 8], v[12], m[s[ 0]], m[s[ 1]]);
        mix(v[1], v[5], v[ 9], v[13], m[s[ 2]], m[s[ 3]]);
        mix(v[2], v[6], v[10], v[14], m[s[ 4]], m[s[ 5]]);
        mix(v[3], v[7], v[11], v[15], m[s[ 6]], m[s[ 7]]);
        mix(v[0], v[5], v[10], v[15], m[s[ 8]], m[s[ 9]]);
        mix(v[1], v[6], v[11], v[12], m[s[10]], m[s[11]]);
        mix(v[2], v[7], v[ 8], v[13], m[s[12]], m[s[13]]);
        mix(v[3], v[4], v[ 9], v[14], m[s[14]], m[s[15]]);
    }
    for (int i = 0; i < 8; ++i) _h[i] ^= v[i] ^ v[i + 8];
}

std::vector<std::uint8_t> Blake2b::oneShot(std::string_view data, std::size_t digest_len)
{
    Blake2b b(digest_len);
    b.update(data);
    return b.finish();
}

std::vector<std::uint8_t> Blake2b::oneShotKeyed(std::string_view data,
                                                std::string_view key,
                                                std::size_t      digest_len)
{
    Blake2b b(digest_len, key);
    b.update(data);
    return b.finish();
}

} // namespace m130::access::crypto

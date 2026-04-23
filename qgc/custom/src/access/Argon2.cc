#include "Argon2.h"

#include "Blake2b.h"

#include <array>
#include <cstring>

namespace m130::access::crypto {

namespace {

constexpr std::size_t kBlockBytes = 1024;
constexpr std::uint32_t kTypeArgon2id = 2;

inline void storeLe32(std::uint8_t* p, std::uint32_t v) noexcept
{
    p[0] = static_cast<std::uint8_t>(v);
    p[1] = static_cast<std::uint8_t>(v >> 8);
    p[2] = static_cast<std::uint8_t>(v >> 16);
    p[3] = static_cast<std::uint8_t>(v >> 24);
}

inline void storeLe64(std::uint8_t* p, std::uint64_t v) noexcept
{
    for (int i = 0; i < 8; ++i) p[i] = static_cast<std::uint8_t>(v >> (8 * i));
}

inline std::uint64_t loadLe64(const std::uint8_t* p) noexcept
{
    std::uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= static_cast<std::uint64_t>(p[i]) << (8 * i);
    return v;
}

inline std::uint64_t rotr64(std::uint64_t x, unsigned n) noexcept
{
    return (x >> n) | (x << (64 - n));
}

// ------ Variable-length hash H' (RFC 9106 §3.3) ------

void hPrime(std::uint32_t out_len, const std::uint8_t* in, std::size_t in_len,
            std::uint8_t* out)
{
    std::uint8_t lenbuf[4];
    storeLe32(lenbuf, out_len);

    if (out_len <= 64) {
        Blake2b b(out_len);
        b.update(lenbuf, 4);
        b.update(in, in_len);
        const auto d = b.finish();
        std::memcpy(out, d.data(), out_len);
        return;
    }

    // r = ceil(out_len / 32) - 2 hashes of 32 bytes, followed by final
    // partial-length hash.
    const std::uint32_t r = (out_len + 31) / 32 - 2;

    Blake2b b(64);
    b.update(lenbuf, 4);
    b.update(in, in_len);
    std::vector<std::uint8_t> v = b.finish(); // 64 bytes

    std::memcpy(out, v.data(), 32);           // first 32 bytes of V1
    std::size_t off = 32;

    for (std::uint32_t i = 2; i <= r; ++i) {
        v = Blake2b::oneShot(
            std::string_view(reinterpret_cast<const char*>(v.data()), v.size()),
            64);
        std::memcpy(out + off, v.data(), 32);
        off += 32;
    }

    // Final hash producing (out_len - 32*r) bytes from the last full V.
    const std::uint32_t tail = out_len - 32 * r;
    Blake2b btail(tail);
    btail.update(v.data(), v.size());
    const auto vlast = btail.finish();
    std::memcpy(out + off, vlast.data(), tail);
}

// ------ GB mixing function and P permutation (RFC 9106 §3.6) ------

inline void gb(std::uint64_t& a, std::uint64_t& b,
               std::uint64_t& c, std::uint64_t& d) noexcept
{
    const auto lo = [](std::uint64_t x) { return x & 0xffffffffULL; };
    a = a + b + 2 * lo(a) * lo(b); d = rotr64(d ^ a, 32);
    c = c + d + 2 * lo(c) * lo(d); b = rotr64(b ^ c, 24);
    a = a + b + 2 * lo(a) * lo(b); d = rotr64(d ^ a, 16);
    c = c + d + 2 * lo(c) * lo(d); b = rotr64(b ^ c, 63);
}

// P permutation on 16 u64 words.
inline void P(std::uint64_t* v) noexcept
{
    gb(v[ 0], v[ 4], v[ 8], v[12]);
    gb(v[ 1], v[ 5], v[ 9], v[13]);
    gb(v[ 2], v[ 6], v[10], v[14]);
    gb(v[ 3], v[ 7], v[11], v[15]);
    gb(v[ 0], v[ 5], v[10], v[15]);
    gb(v[ 1], v[ 6], v[11], v[12]);
    gb(v[ 2], v[ 7], v[ 8], v[13]);
    gb(v[ 3], v[ 4], v[ 9], v[14]);
}

// Compression G (1024-byte blocks).
// result = (P applied to cols of (P applied to rows of R)) XOR R  where R = X^Y.
void compressG(const std::uint8_t* X, const std::uint8_t* Y, std::uint8_t* out,
               bool xor_prev)
{
    std::uint64_t R[128];
    for (int i = 0; i < 128; ++i) {
        R[i] = loadLe64(X + i * 8) ^ loadLe64(Y + i * 8);
    }

    std::uint64_t Q[128];
    std::memcpy(Q, R, sizeof(R));

    // Row-wise P (each row = 16 u64s = 128 bytes).
    for (int i = 0; i < 8; ++i) {
        P(&Q[i * 16]);
    }

    // Column-wise P (each column = 16 u64s gathered across 8 rows,
    // 2 u64s per row per column).
    for (int j = 0; j < 8; ++j) {
        std::uint64_t col[16];
        for (int i = 0; i < 8; ++i) {
            col[i * 2    ] = Q[i * 16 + j * 2    ];
            col[i * 2 + 1] = Q[i * 16 + j * 2 + 1];
        }
        P(col);
        for (int i = 0; i < 8; ++i) {
            Q[i * 16 + j * 2    ] = col[i * 2    ];
            Q[i * 16 + j * 2 + 1] = col[i * 2 + 1];
        }
    }

    // output = Q ^ R (and optionally ^ out for pass > 0).
    for (int i = 0; i < 128; ++i) {
        std::uint64_t w = Q[i] ^ R[i];
        if (xor_prev) w ^= loadLe64(out + i * 8);
        storeLe64(out + i * 8, w);
    }
}

// Reference-index mapping from (J1, J2) for Argon2 (RFC 9106 §3.4.1.2).
std::size_t mapRefIndex(std::uint32_t J1, std::size_t ref_area_size)
{
    const std::uint64_t x  = (static_cast<std::uint64_t>(J1) * J1) >> 32;
    const std::uint64_t y  = (ref_area_size * x) >> 32;
    const std::uint64_t z  = ref_area_size - 1 - y;
    return static_cast<std::size_t>(z);
}

// ---- Argon2i pseudo-random block generator for Argon2id data-indep slices.
struct AddressGen {
    std::uint32_t lane = 0;
    std::uint32_t slice = 0;
    std::uint32_t pass = 0;
    std::uint32_t mprime = 0;
    std::uint32_t total_iterations = 0;
    std::uint64_t counter = 0;
    std::array<std::uint8_t, kBlockBytes> input{};
    std::array<std::uint8_t, kBlockBytes> zero{};
    std::array<std::uint8_t, kBlockBytes> address{};
    std::array<std::uint8_t, kBlockBytes> tmp{};

    void init(std::uint32_t p_, std::uint32_t l_, std::uint32_t s_,
              std::uint32_t m_, std::uint32_t t_)
    {
        pass = p_; lane = l_; slice = s_;
        mprime = m_; total_iterations = t_;
        counter = 0;
        input.fill(0);
        zero.fill(0);
        storeLe64(input.data() +  0, pass);
        storeLe64(input.data() +  8, lane);
        storeLe64(input.data() + 16, slice);
        storeLe64(input.data() + 24, mprime);
        storeLe64(input.data() + 32, total_iterations);
        storeLe64(input.data() + 40, kTypeArgon2id);
        // Counter slot stays at offset 48 (written by `next()`).
        refresh();
    }

    void refresh()
    {
        ++counter;
        storeLe64(input.data() + 48, counter);
        compressG(zero.data(), input.data(), tmp.data(), false);
        compressG(zero.data(), tmp.data(), address.data(), false);
    }
};

// ------ Build H0 seed (RFC 9106 §3.2) ------

std::vector<std::uint8_t> buildH0(std::string_view pwd, std::string_view salt,
                                  std::string_view secret, std::string_view assoc,
                                  const Argon2Params& p)
{
    Blake2b b(64);
    std::uint8_t buf[4];
    storeLe32(buf, p.parallelism);  b.update(buf, 4);
    storeLe32(buf, p.tag_length);   b.update(buf, 4);
    storeLe32(buf, p.memory_kib);   b.update(buf, 4);
    storeLe32(buf, p.iterations);   b.update(buf, 4);
    storeLe32(buf, Argon2Params::kVersion); b.update(buf, 4);
    storeLe32(buf, kTypeArgon2id);  b.update(buf, 4);

    storeLe32(buf, static_cast<std::uint32_t>(pwd.size()));   b.update(buf, 4);
    b.update(pwd);
    storeLe32(buf, static_cast<std::uint32_t>(salt.size()));  b.update(buf, 4);
    b.update(salt);
    storeLe32(buf, static_cast<std::uint32_t>(secret.size())); b.update(buf, 4);
    b.update(secret);
    storeLe32(buf, static_cast<std::uint32_t>(assoc.size()));  b.update(buf, 4);
    b.update(assoc);

    return b.finish();
}

} // namespace

Argon2Error validateParams(const Argon2Params& p) noexcept
{
    if (p.parallelism < 1)                 return Argon2Error::BadParallelism;
    if (p.iterations  < 1)                 return Argon2Error::BadIterations;
    if (p.tag_length  < 4)                 return Argon2Error::BadTagLength;
    if (p.memory_kib  < 8 * p.parallelism) return Argon2Error::BadMemory;
    return Argon2Error::Ok;
}

std::vector<std::uint8_t> argon2idHash(std::string_view password,
                                       std::string_view salt,
                                       std::string_view secret,
                                       std::string_view associated,
                                       const Argon2Params& p)
{
    if (validateParams(p) != Argon2Error::Ok) return {};

    // Round m down to a multiple of 4*p.
    const std::uint32_t lanes       = p.parallelism;
    const std::uint32_t m_prime     = (p.memory_kib / (4 * lanes)) * 4 * lanes;
    const std::uint32_t lane_length = m_prime / lanes;
    const std::uint32_t seg_length  = lane_length / 4;

    // Memory: `m_prime` blocks, each kBlockBytes.
    std::vector<std::uint8_t> mem(static_cast<std::size_t>(m_prime) * kBlockBytes, 0);
    auto block = [&](std::uint32_t lane, std::uint32_t index) -> std::uint8_t* {
        return mem.data() + (static_cast<std::size_t>(lane) * lane_length + index) * kBlockBytes;
    };

    // H0 seed.
    const auto H0 = buildH0(password, salt, secret, associated, p);

    // Initial blocks B[lane][0] and B[lane][1].
    for (std::uint32_t lane = 0; lane < lanes; ++lane) {
        std::uint8_t in[64 + 8];
        std::memcpy(in, H0.data(), 64);
        storeLe32(in + 64, 0);
        storeLe32(in + 68, lane);
        hPrime(kBlockBytes, in, sizeof(in), block(lane, 0));
        storeLe32(in + 64, 1);
        hPrime(kBlockBytes, in, sizeof(in), block(lane, 1));
    }

    // Fill remaining blocks.
    AddressGen ag;
    for (std::uint32_t it = 0; it < p.iterations; ++it) {
        for (std::uint32_t slice = 0; slice < 4; ++slice) {
            // Argon2id uses Argon2i (data-independent) mode during pass 0
            // slices 0 and 1; Argon2d (data-dependent) otherwise.
            const bool data_independent = (it == 0 && slice < 2);

            for (std::uint32_t lane = 0; lane < lanes; ++lane) {
                if (data_independent) {
                    ag.init(it, lane, slice, m_prime, p.iterations);
                }

                const std::uint32_t start_idx =
                    (it == 0 && slice == 0) ? 2 : 0;

                for (std::uint32_t i = start_idx; i < seg_length; ++i) {
                    const std::uint32_t j = slice * seg_length + i;
                    const std::uint32_t prev_idx =
                        (j == 0) ? (lane_length - 1) : (j - 1);
                    const std::uint8_t* prev = block(lane, prev_idx);

                    // Get J1, J2 — from prev block (d-mode) or address block
                    // (i-mode). Address block holds 128 pairs of (J1, J2)
                    // each 8 bytes; refresh every 128 lookups.
                    std::uint32_t J1 = 0, J2 = 0;
                    if (data_independent) {
                        const std::uint32_t local_idx = i % 128;
                        if (i != start_idx && local_idx == 0) {
                            ag.refresh();
                        } else if (i == start_idx) {
                            // First use — already refreshed in init().
                        }
                        const std::uint8_t* ap = ag.address.data() + local_idx * 8;
                        J1 = static_cast<std::uint32_t>(loadLe64(ap));
                        J2 = static_cast<std::uint32_t>(loadLe64(ap) >> 32);
                    } else {
                        const std::uint64_t w = loadLe64(prev);
                        J1 = static_cast<std::uint32_t>(w);
                        J2 = static_cast<std::uint32_t>(w >> 32);
                    }

                    // Compute reference lane and index per RFC 9106 §3.4.1.1.
                    std::uint32_t ref_lane = (it == 0 && slice == 0)
                        ? lane : (J2 % lanes);

                    std::size_t ref_area;
                    if (it == 0) {
                        if (slice == 0 || ref_lane == lane) {
                            // Same lane — reference area is [0, j-1].
                            ref_area = j - 1;
                        } else {
                            // Prior slices of other lane, excluding current i=0.
                            ref_area = slice * seg_length - (i == 0 ? 1 : 0);
                        }
                    } else {
                        if (ref_lane == lane) {
                            ref_area = lane_length - seg_length + i - 1;
                        } else {
                            ref_area = lane_length - seg_length - (i == 0 ? 1 : 0);
                        }
                    }

                    const std::size_t rel_idx = mapRefIndex(J1, ref_area);
                    std::size_t start_pos = 0;
                    if (it != 0) start_pos = (slice + 1) * seg_length;
                    const std::size_t ref_index =
                        (start_pos + rel_idx) % lane_length;

                    const std::uint8_t* refblk = block(ref_lane, static_cast<std::uint32_t>(ref_index));
                    const bool xor_prev = (it != 0);
                    compressG(prev, refblk, block(lane, j), xor_prev);
                }
            }
        }
    }

    // Final XOR across lanes of last block, then H' to tag.
    std::vector<std::uint8_t> final_block(kBlockBytes, 0);
    for (std::uint32_t lane = 0; lane < lanes; ++lane) {
        const std::uint8_t* b = block(lane, lane_length - 1);
        for (std::size_t i = 0; i < kBlockBytes; ++i) final_block[i] ^= b[i];
    }

    std::vector<std::uint8_t> tag(p.tag_length);
    hPrime(p.tag_length, final_block.data(), final_block.size(), tag.data());
    return tag;
}

std::vector<std::uint8_t> argon2idHash(std::string_view password,
                                       std::string_view salt,
                                       const Argon2Params& p)
{
    return argon2idHash(password, salt, {}, {}, p);
}

} // namespace m130::access::crypto

#include "access/Blake2b.h"
#include "test_support.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using m130::access::crypto::Blake2b;

namespace {

std::string toHex(const std::vector<std::uint8_t>& v)
{
    static const char* k = "0123456789abcdef";
    std::string out;
    out.reserve(v.size() * 2);
    for (auto b : v) { out.push_back(k[b >> 4]); out.push_back(k[b & 0xf]); }
    return out;
}

} // namespace

// RFC 7693 Appendix A test vector: BLAKE2b-512 of "abc".
int abcVector()
{
    const auto d = Blake2b::oneShot("abc", 64);
    M130_REQUIRE_EQ(toHex(d),
        std::string("ba80a53f981c4d0d6a2797b69f12f6e9"
                    "4c212f14685ac4b74b12bb6fdbffa2d1"
                    "7d87c5392aab792dc252d5de4533cc95"
                    "18d38aa8dbf1925ab92386edd4009923"));
    return 0;
}

// Empty input, 512-bit digest.
int emptyVector()
{
    const auto d = Blake2b::oneShot("", 64);
    M130_REQUIRE_EQ(toHex(d),
        std::string("786a02f742015903c6c6fd852552d272"
                    "912f4740e15847618a86e217f71f5419"
                    "d25e1031afee585313896444934eb04b"
                    "903a685b1448b755d56f701afe9be2ce"));
    return 0;
}

// Reduced digest size still obeys prefix-of-full: BLAKE2b-256 of "abc".
int shortDigestVector()
{
    const auto d = Blake2b::oneShot("abc", 32);
    M130_REQUIRE_EQ(toHex(d),
        std::string("bddd813c634239723171ef3fee98579b"
                    "94964e3bb1cb3e427262c8c068d52319"));
    return 0;
}

// Keyed BLAKE2b: key=0x00..0x3f, data=empty → matches libsodium reference.
int keyedEmptyVector()
{
    std::string key(64, '\0');
    for (std::size_t i = 0; i < key.size(); ++i) key[i] = static_cast<char>(i);
    const auto d = Blake2b::oneShotKeyed("", key, 64);
    M130_REQUIRE_EQ(toHex(d),
        std::string("10ebb67700b1868efb4417987acf4690"
                    "ae9d972fb7a590c2f02871799aaa4786"
                    "b5e996e8f0f4eb981fc214b005f42d2f"
                    "f4233499391653df7aefcbc13fc51568"));
    return 0;
}

// Incremental update must match a single-shot hash.
int incrementalMatchesOneShot()
{
    const std::string s =
        "The quick brown fox jumps over the lazy dog";
    const auto one = Blake2b::oneShot(s, 64);

    Blake2b b(64);
    b.update(reinterpret_cast<const std::uint8_t*>(s.data()), 5);
    b.update(reinterpret_cast<const std::uint8_t*>(s.data() + 5), s.size() - 5);
    const auto inc = b.finish();
    M130_REQUIRE(one == inc);
    return 0;
}

// Digest length must fit 1..64.
int rejectsBadDigestLen()
{
    try { Blake2b b(0);  return 1; } catch (const std::invalid_argument&) {}
    try { Blake2b b(65); return 1; } catch (const std::invalid_argument&) {}
    return 0;
}

int run()
{
    M130_RUN(abcVector);
    M130_RUN(emptyVector);
    M130_RUN(shortDigestVector);
    M130_RUN(keyedEmptyVector);
    M130_RUN(incrementalMatchesOneShot);
    M130_RUN(rejectsBadDigestLen);
    return 0;
}

M130_TEST_MAIN()

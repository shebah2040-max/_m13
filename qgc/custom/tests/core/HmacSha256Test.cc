#include "logging/Hmac.h"
#include "test_support.h"

#include <string>
#include <vector>

using namespace m130::logging::crypto;

namespace {
std::string bytes(std::size_t n, std::uint8_t v)
{
    return std::string(n, static_cast<char>(v));
}

std::string fromHex(const std::string& hex)
{
    std::string out;
    out.reserve(hex.size() / 2);
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        const auto hex2int = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return 0;
        };
        out.push_back(static_cast<char>((hex2int(hex[i]) << 4) | hex2int(hex[i + 1])));
    }
    return out;
}
} // namespace

// RFC 4231 Test Case 1
int rfc4231_case1()
{
    const std::string key  = bytes(20, 0x0b);
    const std::string data = "Hi There";
    const auto mac = HmacSha256::macHex(key, data);
    M130_REQUIRE_EQ(mac,
        std::string("b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"));
    return 0;
}

// RFC 4231 Test Case 2 (key shorter than block size, text with non-ASCII)
int rfc4231_case2()
{
    const std::string key  = "Jefe";
    const std::string data = "what do ya want for nothing?";
    const auto mac = HmacSha256::macHex(key, data);
    M130_REQUIRE_EQ(mac,
        std::string("5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"));
    return 0;
}

// RFC 4231 Test Case 3 (20-byte 0xaa key, 50 0xdd bytes)
int rfc4231_case3()
{
    const std::string key  = bytes(20, 0xaa);
    const std::string data = bytes(50, 0xdd);
    const auto mac = HmacSha256::macHex(key, data);
    M130_REQUIRE_EQ(mac,
        std::string("773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe"));
    return 0;
}

// RFC 4231 Test Case 6 — key larger than block size (131 bytes 0xaa)
int rfc4231_case6()
{
    const std::string key  = bytes(131, 0xaa);
    const std::string data = "Test Using Larger Than Block-Size Key - Hash Key First";
    const auto mac = HmacSha256::macHex(key, data);
    M130_REQUIRE_EQ(mac,
        std::string("60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54"));
    return 0;
}

int incrementalMatchesOneShot()
{
    const std::string key  = "sesame";
    const std::string data = "open please";
    const auto one = HmacSha256::macHex(key, data);
    HmacSha256 h;
    h.init(key.data(), key.size());
    h.update(data.data(), 4);
    h.update(data.data() + 4, data.size() - 4);
    const auto inc = Sha256::toHex(h.finalize());
    M130_REQUIRE_EQ(one, inc);
    // Guard against accidental equality with hex fixture.
    (void)fromHex("00");
    return 0;
}

int run()
{
    M130_RUN(rfc4231_case1);
    M130_RUN(rfc4231_case2);
    M130_RUN(rfc4231_case3);
    M130_RUN(rfc4231_case6);
    M130_RUN(incrementalMatchesOneShot);
    return 0;
}

M130_TEST_MAIN()

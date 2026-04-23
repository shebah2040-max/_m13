#include "access/Totp.h"
#include "test_support.h"

using namespace m130::access::crypto;

namespace {

// RFC 6238 Appendix B SHA-256 variant uses the 32-byte seed:
//   "12345678901234567890123456789012"
// The expected codes at the given Unix timestamps (8 digits) come directly
// from the RFC. We store them for reference and also run 6-digit checks.
constexpr const char* kSecretSha256 = "12345678901234567890123456789012";

struct Vec {
    std::uint64_t t_s;
    const char*   code8;
};

// RFC 6238 Appendix B, algorithm=SHA256.
constexpr Vec kRfc6238Sha256[] = {
    {         59ull, "46119246" },
    { 1111111109ull, "68084774" },
    { 1111111111ull, "67062674" },
    { 1234567890ull, "91819424" },
    { 2000000000ull, "90698825" },
};

} // namespace

int rfc6238Sha256TestVectors()
{
    for (const auto& v : kRfc6238Sha256) {
        const std::string got = Totp::totp(kSecretSha256, v.t_s, 30, 8);
        M130_REQUIRE_EQ(got, std::string(v.code8));
    }
    return 0;
}

int verifyAcceptsCurrentAndNeighbouringWindows()
{
    constexpr std::uint64_t t = 1234567890;
    const std::string code = Totp::totp(kSecretSha256, t, 30, 6);
    M130_REQUIRE(Totp::verify(kSecretSha256, t,          code));
    M130_REQUIRE(Totp::verify(kSecretSha256, t + 15,     code));
    M130_REQUIRE(Totp::verify(kSecretSha256, t - 29,     code));
    // +2 windows (60s forward) must be rejected with default lookback=1.
    M130_REQUIRE(!Totp::verify(kSecretSha256, t + 90,    code));
    return 0;
}

int verifyRejectsWrongCode()
{
    const std::uint64_t t = 1234567890;
    M130_REQUIRE(!Totp::verify(kSecretSha256, t, "000000"));
    M130_REQUIRE(!Totp::verify(kSecretSha256, t, ""));
    M130_REQUIRE(!Totp::verify(kSecretSha256, t, "1"));
    return 0;
}

int hotpIsZeroPadded()
{
    // Any counter value that yields < 10^(digits-1) must be zero-padded.
    std::string c = Totp::hotp(kSecretSha256, 0, 6);
    M130_REQUIRE_EQ(c.size(), 6u);
    for (char ch : c) M130_REQUIRE(ch >= '0' && ch <= '9');
    return 0;
}

int run()
{
    M130_RUN(rfc6238Sha256TestVectors);
    M130_RUN(verifyAcceptsCurrentAndNeighbouringWindows);
    M130_RUN(verifyRejectsWrongCode);
    M130_RUN(hotpIsZeroPadded);
    return 0;
}

M130_TEST_MAIN()

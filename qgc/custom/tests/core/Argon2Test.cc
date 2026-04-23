#include "access/Argon2.h"
#include "access/PasswordHasher.h"
#include "test_support.h"

#include <cstdint>
#include <string>
#include <vector>

using namespace m130::access::crypto;

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

// RFC 9106 §A.3 — Argon2id test vector.
//   P = 32 bytes of 0x01
//   S = 16 bytes of 0x02
//   K =  8 bytes of 0x03 (secret)
//   X = 12 bytes of 0x04 (associated data)
//   p=4, t=3, m=32 KiB, τ=32, v=0x13
//   Tag = 0d 64 0d f5 8d 78 76 6c 08 c0 37 a3 4a 8b 53 c9
//         d0 1e f0 45 2d 75 b6 5e b5 25 20 e9 6b 01 e6 59
int rfc9106Vector()
{
    std::string pwd(32, '\x01');
    std::string salt(16, '\x02');
    std::string secret(8, '\x03');
    std::string assoc(12, '\x04');

    Argon2Params p;
    p.memory_kib  = 32;
    p.iterations  = 3;
    p.parallelism = 4;
    p.tag_length  = 32;

    const auto tag = argon2idHash(pwd, salt, secret, assoc, p);
    M130_REQUIRE_EQ(toHex(tag),
        std::string("0d640df58d78766c08c037a34a8b53c9"
                    "d01ef0452d75b65eb52520e96b01e659"));
    return 0;
}

// Minimal parameters: m=8, t=1, p=1, τ=32, no secret/assoc.
int smallParamsRoundtrip()
{
    Argon2Params p;
    p.memory_kib  = 8;
    p.iterations  = 1;
    p.parallelism = 1;
    p.tag_length  = 32;

    const auto tag1 = argon2idHash("passw0rd!", "saltsalt", p);
    const auto tag2 = argon2idHash("passw0rd!", "saltsalt", p);
    M130_REQUIRE(tag1 == tag2);
    M130_REQUIRE(tag1.size() == 32);

    const auto tag3 = argon2idHash("WRONGpass", "saltsalt", p);
    M130_REQUIRE(tag1 != tag3);
    return 0;
}

int validatesParams()
{
    Argon2Params p;
    p.parallelism = 0;
    M130_REQUIRE(validateParams(p) == Argon2Error::BadParallelism);

    p.parallelism = 2;
    p.memory_kib  = 8; // < 8*p=16
    M130_REQUIRE(validateParams(p) == Argon2Error::BadMemory);

    p.memory_kib  = 32;
    p.iterations  = 0;
    M130_REQUIRE(validateParams(p) == Argon2Error::BadIterations);

    p.iterations = 1;
    p.tag_length = 3;
    M130_REQUIRE(validateParams(p) == Argon2Error::BadTagLength);

    p.tag_length = 16;
    M130_REQUIRE(validateParams(p) == Argon2Error::Ok);
    return 0;
}

int hasherRoundtrip()
{
    // Keep m small so the test stays fast.
    m130::access::Argon2idHasher h(/*m=*/16, /*t=*/1, /*p=*/1);
    const std::string salt = "8-byte-s";
    const auto enc = h.hash("correct-horse-battery-staple", salt);
    M130_REQUIRE(enc.rfind("argon2id$v=19$m=16,t=1,p=1$", 0) == 0);
    M130_REQUIRE(h.verify("correct-horse-battery-staple", enc));
    M130_REQUIRE(!h.verify("wrong-password",              enc));
    return 0;
}

int hasherRejectsMalformedEncoding()
{
    m130::access::Argon2idHasher h;
    M130_REQUIRE(!h.verify("x", "argon2id$v=19$m=8,t=1,p=1$salt"));          // missing hash
    M130_REQUIRE(!h.verify("x", "argon2id$v=18$m=8,t=1,p=1$c2FsdA$YWJj"));   // bad version
    M130_REQUIRE(!h.verify("x", "pbkdf2-sha256$i=1000$c2FsdA$YWJj"));        // wrong scheme
    return 0;
}

int run()
{
    M130_RUN(rfc9106Vector);
    M130_RUN(smallParamsRoundtrip);
    M130_RUN(validatesParams);
    M130_RUN(hasherRoundtrip);
    M130_RUN(hasherRejectsMalformedEncoding);
    return 0;
}

M130_TEST_MAIN()

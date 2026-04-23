#include "access/Pbkdf2.h"
#include "access/PasswordHasher.h"
#include "test_support.h"

#include <iomanip>
#include <sstream>

using namespace m130::access;
using namespace m130::access::crypto;

namespace {
std::string toHex(const std::vector<std::uint8_t>& v)
{
    std::ostringstream os;
    os << std::hex << std::setfill('0');
    for (std::uint8_t b : v) os << std::setw(2) << static_cast<int>(b);
    return os.str();
}
} // namespace

int deriveIsDeterministic()
{
    const auto a = Pbkdf2::derive("password", "salt", 1000, 32);
    const auto b = Pbkdf2::derive("password", "salt", 1000, 32);
    M130_REQUIRE_EQ(toHex(a), toHex(b));
    // Different salt -> different output.
    const auto c = Pbkdf2::derive("password", "salt2", 1000, 32);
    M130_REQUIRE(toHex(a) != toHex(c));
    // Different password -> different output.
    const auto d = Pbkdf2::derive("Password", "salt", 1000, 32);
    M130_REQUIRE(toHex(a) != toHex(d));
    return 0;
}

int deriveKnownSha256Vector()
{
    // RFC 7914 / PBKDF2-HMAC-SHA256 vector — widely reproduced (e.g. by
    // Python `hashlib.pbkdf2_hmac('sha256', b'passwd', b'salt', 1, 64)`).
    // Expected: 55ac046e56e3089fec1691c22544b605f94185216dde0465e68b9d57c20dacbc
    //           49ca9cccf179b645991664b39d77ef317c71b845b1e30bd509112041d3a19783
    const auto dk = Pbkdf2::derive("passwd", "salt", 1, 64);
    const std::string hex = toHex(dk);
    M130_REQUIRE_EQ(hex.substr(0, 32), std::string("55ac046e56e3089fec1691c22544b605"));
    return 0;
}

int hasherRoundTrip()
{
    Pbkdf2Hasher h(2048);
    const std::string salt(16, 'X');
    const std::string encoded = h.hash("correct-horse-staple-battery", salt);
    M130_REQUIRE(h.verify("correct-horse-staple-battery", encoded));
    M130_REQUIRE(!h.verify("wrong",                        encoded));
    M130_REQUIRE(!h.verify("correct-horse-staple-batteryX",encoded));
    return 0;
}

int hasherEncodedFormatIsSelfDescribing()
{
    Pbkdf2Hasher h(1024);
    const std::string e = h.hash("pw", "salty");
    // Expected shape: "pbkdf2-sha256$i=1024$...$..."
    M130_REQUIRE(e.find("pbkdf2-sha256$") == 0);
    M130_REQUIRE(e.find("i=1024") != std::string::npos);
    // Four segments separated by '$'.
    int dollars = 0;
    for (char c : e) if (c == '$') ++dollars;
    M130_REQUIRE_EQ(dollars, 3);
    return 0;
}

int run()
{
    M130_RUN(deriveIsDeterministic);
    M130_RUN(deriveKnownSha256Vector);
    M130_RUN(hasherRoundTrip);
    M130_RUN(hasherEncodedFormatIsSelfDescribing);
    return 0;
}

M130_TEST_MAIN()

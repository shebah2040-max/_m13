#include "access/Base32.h"
#include "test_support.h"

#include <string>

using namespace m130::access::crypto;

namespace {
std::string bytesToString(const std::vector<std::uint8_t>& b)
{
    return std::string(b.begin(), b.end());
}
} // namespace

int rfc4648TestVectors()
{
    // RFC 4648 §10 test vectors.
    M130_REQUIRE_EQ(Base32::encode(""), std::string(""));
    M130_REQUIRE_EQ(Base32::encode("f"), std::string("MY======"));
    M130_REQUIRE_EQ(Base32::encode("fo"), std::string("MZXQ===="));
    M130_REQUIRE_EQ(Base32::encode("foo"), std::string("MZXW6==="));
    M130_REQUIRE_EQ(Base32::encode("foob"), std::string("MZXW6YQ="));
    M130_REQUIRE_EQ(Base32::encode("fooba"), std::string("MZXW6YTB"));
    M130_REQUIRE_EQ(Base32::encode("foobar"), std::string("MZXW6YTBOI======"));
    return 0;
}

int decodeRoundTrip()
{
    auto roundTrip = [](const std::string& s) {
        auto e = Base32::encode(s);
        auto d = Base32::decode(e);
        M130_REQUIRE(d.has_value());
        M130_REQUIRE_EQ(bytesToString(*d), s);
        return 0;
    };
    for (const char* s : {"", "f", "fo", "foo", "foob", "fooba", "foobar",
                          "Hello, World!", "The quick brown fox"}) {
        if (roundTrip(s) != 0) return 1;
    }
    return 0;
}

int decodeIsCaseInsensitive()
{
    auto up = Base32::decode("MZXW6YTBOI======");
    auto lo = Base32::decode("mzxw6ytboi======");
    M130_REQUIRE(up.has_value() && lo.has_value());
    M130_REQUIRE_EQ(bytesToString(*up), std::string("foobar"));
    M130_REQUIRE_EQ(bytesToString(*lo), std::string("foobar"));
    return 0;
}

int decodeRejectsInvalidAlphabet()
{
    M130_REQUIRE(!Base32::decode("1!@#").has_value());
    M130_REQUIRE(!Base32::decode("MZXW0YTB").has_value()); // '0' is invalid
    return 0;
}

int run()
{
    M130_RUN(rfc4648TestVectors);
    M130_RUN(decodeRoundTrip);
    M130_RUN(decodeIsCaseInsensitive);
    M130_RUN(decodeRejectsInvalidAlphabet);
    return 0;
}

M130_TEST_MAIN()

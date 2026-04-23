#include "logging/Sha256.h"
#include "test_support.h"

#include <string>

using namespace m130::logging::crypto;

int emptyVector()
{
    const auto d = Sha256::hash("", 0);
    M130_REQUIRE_EQ(Sha256::toHex(d),
        std::string("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    return 0;
}

int abcVector()
{
    const auto d = Sha256::hash("abc", 3);
    M130_REQUIRE_EQ(Sha256::toHex(d),
        std::string("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
    return 0;
}

int twoBlockVector()
{
    // 56 bytes → exactly hits the 56-byte padding boundary (2 blocks).
    const std::string s = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const auto d = Sha256::hash(s);
    M130_REQUIRE_EQ(Sha256::toHex(d),
        std::string("248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"));
    return 0;
}

int million_aVector()
{
    Sha256 h;
    for (int i = 0; i < 1000000; ++i) {
        const char c = 'a';
        h.update(&c, 1);
    }
    const auto d = h.finalize();
    M130_REQUIRE_EQ(Sha256::toHex(d),
        std::string("cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"));
    return 0;
}

int incrementalMatchesOneShot()
{
    const std::string s = "The quick brown fox jumps over the lazy dog";
    const auto one = Sha256::hash(s);
    Sha256 h;
    h.update(s.data(), 10);
    h.update(s.data() + 10, s.size() - 10);
    const auto inc = h.finalize();
    M130_REQUIRE(one == inc);
    return 0;
}

int run()
{
    M130_RUN(emptyVector);
    M130_RUN(abcVector);
    M130_RUN(twoBlockVector);
    M130_RUN(million_aVector);
    M130_RUN(incrementalMatchesOneShot);
    return 0;
}

M130_TEST_MAIN()

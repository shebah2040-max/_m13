#include "logging/ChainOfCustody.h"
#include "test_support.h"

#include <filesystem>
#include <fstream>

using namespace m130::logging;

namespace {
std::string writeTempFile(const std::string& contents)
{
    auto p = std::filesystem::temp_directory_path() / "m130_coc_test.bin";
    std::ofstream o(p.string(), std::ios::binary);
    o << contents;
    return p.string();
}
} // namespace

int signAndVerify()
{
    ChainOfCustody c;
    c.registerKey("k1", "secret1");
    auto path = writeTempFile("hello world");
    auto m = c.signFile(path);
    M130_REQUIRE(m.has_value());
    M130_REQUIRE(c.verifyFile(*m));
    // Tamper.
    std::ofstream o(path, std::ios::trunc | std::ios::binary);
    o << "HELLO world";
    o.close();
    M130_REQUIRE(!c.verifyFile(*m));
    std::filesystem::remove(path);
    return 0;
}

int chainLinks()
{
    ChainOfCustody c;
    c.registerKey("k1", "x");
    auto path = writeTempFile("a");
    auto m1 = *c.signFile(path);
    auto m2 = *c.signFile(path);
    auto c1 = c.chain(m1);
    auto c2 = c.chain(m2);
    M130_REQUIRE_EQ(c2.prev_manifest_hash, c1.this_manifest_hash);
    std::filesystem::remove(path);
    return 0;
}

int keyRotation()
{
    ChainOfCustody c;
    c.registerKey("k1", "v1");
    c.registerKey("k2", "v2");
    auto path = writeTempFile("data");
    auto m = *c.signFile(path);
    M130_REQUIRE_EQ(m.key_id, std::string("k2")); // last registered is active
    c.setActiveKey("k1");
    auto m2 = *c.signFile(path);
    M130_REQUIRE_EQ(m2.key_id, std::string("k1"));
    std::filesystem::remove(path);
    return 0;
}

int run()
{
    M130_RUN(signAndVerify);
    M130_RUN(chainLinks);
    M130_RUN(keyRotation);
    return 0;
}

M130_TEST_MAIN()

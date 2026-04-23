#include "safety/AlertManager.h"
#include "test_support.h"

using namespace m130::safety;

namespace { uint64_t g = 0; uint64_t clk() { return g += 1; } }

int raiseAndEscalate()
{
    AlertManager am(&clk, 256);
    M130_REQUIRE(am.raise("x", AlertLevel::Caution, "hi"));
    M130_REQUIRE(am.raise("x", AlertLevel::Warning, "upgraded"));
    M130_REQUIRE_EQ(am.masterLevel(), AlertLevel::Warning);
    // Downgrade ignored.
    M130_REQUIRE_EQ(am.raise("x", AlertLevel::Caution, "down"), false);
    M130_REQUIRE_EQ(am.masterLevel(), AlertLevel::Warning);
    return 0;
}

int acknowledge()
{
    AlertManager am(&clk, 256);
    am.raise("a", AlertLevel::Caution, "a");
    M130_REQUIRE(am.acknowledge("a", "rso"));
    M130_REQUIRE_EQ(am.masterLevel(), AlertLevel::None);
    // Cannot ack twice.
    M130_REQUIRE_EQ(am.acknowledge("a", "rso"), false);
    return 0;
}

int multipleMasterLevel()
{
    AlertManager am(&clk, 256);
    am.raise("a", AlertLevel::Advisory, "a");
    am.raise("b", AlertLevel::Warning, "b");
    am.raise("c", AlertLevel::Caution, "c");
    M130_REQUIRE_EQ(am.masterLevel(), AlertLevel::Warning);
    return 0;
}

int sortedActive()
{
    AlertManager am(&clk, 256);
    am.raise("a", AlertLevel::Advisory, "a");
    am.raise("b", AlertLevel::Emergency, "b");
    am.raise("c", AlertLevel::Caution, "c");
    auto v = am.active();
    M130_REQUIRE_EQ(v.size(), std::size_t(3));
    M130_REQUIRE_EQ(v[0].level, AlertLevel::Emergency);
    return 0;
}

int floodEvictsLowestSeverity()
{
    AlertManager am(&clk, /*cap=*/3);
    am.raise("a", AlertLevel::Advisory, "a");
    am.raise("b", AlertLevel::Caution, "b");
    am.raise("c", AlertLevel::Warning, "c");
    am.raise("d", AlertLevel::Emergency, "d");
    // one must have been evicted — advisory "a" should go first
    auto v = am.active();
    M130_REQUIRE_EQ(v.size(), std::size_t(3));
    for (const auto& a : v) M130_EXPECT(a.id != "a");
    return 0;
}

int subscribersCalled()
{
    AlertManager am(&clk, 256);
    int raises = 0, acks = 0;
    am.subscribe([&](const Alert&, bool is_ack) { is_ack ? ++acks : ++raises; });
    am.raise("a", AlertLevel::Warning, "a");
    am.acknowledge("a", "user");
    M130_REQUIRE_EQ(raises, 1);
    M130_REQUIRE_EQ(acks, 1);
    return 0;
}

int run()
{
    M130_RUN(raiseAndEscalate);
    M130_RUN(acknowledge);
    M130_RUN(multipleMasterLevel);
    M130_RUN(sortedActive);
    M130_RUN(floodEvictsLowestSeverity);
    M130_RUN(subscribersCalled);
    return 0;
}

M130_TEST_MAIN()

#include "access/SessionManager.h"
#include "test_support.h"

using namespace m130::access;

namespace { uint64_t g = 0; uint64_t clk() { return g; } }

int createAndTouchValidates()
{
    g = 1000;
    SessionManager sm(&clk, nullptr);
    sm.setDefaultTtlMs(5000);
    auto id = sm.create("u", Role::Operator);
    M130_REQUIRE_EQ(sm.state(id), SessionState::Valid);
    g = 2000;
    auto s = sm.touch(id);
    M130_REQUIRE(s.has_value());
    M130_REQUIRE_EQ(s->last_active_ms, std::uint64_t(2000));
    return 0;
}

int expiresNaturally()
{
    g = 0;
    SessionManager sm(&clk, nullptr);
    sm.setDefaultTtlMs(100);
    auto id = sm.create("u", Role::Operator);
    g = 500;
    M130_REQUIRE_EQ(sm.state(id), SessionState::Expired);
    M130_REQUIRE(!sm.touch(id).has_value());
    return 0;
}

int revokedIsRevoked()
{
    g = 0;
    SessionManager sm(&clk, nullptr);
    auto id = sm.create("u", Role::Observer);
    sm.revoke(id);
    M130_REQUIRE_EQ(sm.state(id), SessionState::Revoked);
    return 0;
}

int evictExpired()
{
    g = 0;
    SessionManager sm(&clk, nullptr);
    sm.setDefaultTtlMs(100);
    for (int i = 0; i < 5; ++i) sm.create("u", Role::Observer);
    g = 1000;
    auto n = sm.evictExpired();
    M130_REQUIRE_EQ(n, std::size_t(5));
    M130_REQUIRE_EQ(sm.size(), std::size_t(0));
    return 0;
}

int run()
{
    M130_RUN(createAndTouchValidates);
    M130_RUN(expiresNaturally);
    M130_RUN(revokedIsRevoked);
    M130_RUN(evictExpired);
    return 0;
}

M130_TEST_MAIN()

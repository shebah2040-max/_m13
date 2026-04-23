#include "safety/FlightTerminationService.h"
#include "test_support.h"

using namespace m130::safety;
using namespace m130::access;

namespace {
uint64_t g = 0;
uint64_t clk() { return g; }
bool alwaysYes(std::string_view, std::string_view) { return true; }
bool alwaysNo (std::string_view, std::string_view) { return false; }
} // namespace

int rejectsLowRoles()
{
    int dispatched = 0;
    FlightTerminationService fts(&alwaysYes, [&](const FtsRequest&){ ++dispatched; }, &clk);
    fts.setPhase(FlightPhase::Boost);
    FtsRequest r;
    r.primary_role = Role::Operator;
    r.secondary_role = Role::Operator;
    r.primary_user = "a"; r.secondary_user = "b";
    g = 1000; r.timestamp_ms = 1000;
    auto d = fts.armAndFire(r);
    M130_REQUIRE_EQ(d.result, FtsResult::RejectedRoles);
    M130_REQUIRE_EQ(dispatched, 0);
    return 0;
}

int rejectsSameUser()
{
    FlightTerminationService fts(&alwaysYes, nullptr, &clk);
    fts.setPhase(FlightPhase::Boost);
    FtsRequest r;
    r.primary_role = Role::RangeSafety;
    r.secondary_role = Role::SafetyOfficer;
    r.primary_user = "same"; r.secondary_user = "same";
    g = 1000; r.timestamp_ms = 1000;
    M130_REQUIRE_EQ(fts.armAndFire(r).result, FtsResult::RejectedSameUser);
    return 0;
}

int rejectsOnGround()
{
    FlightTerminationService fts(&alwaysYes, nullptr, &clk);
    fts.setPhase(FlightPhase::Idle);
    FtsRequest r;
    r.primary_role = Role::RangeSafety;
    r.secondary_role = Role::SafetyOfficer;
    r.primary_user = "a"; r.secondary_user = "b";
    g = 1000; r.timestamp_ms = 1000;
    M130_REQUIRE_EQ(fts.armAndFire(r).result, FtsResult::RejectedNotArmed);
    return 0;
}

int rejectsExpired()
{
    FlightTerminationService fts(&alwaysYes, nullptr, &clk);
    fts.setPhase(FlightPhase::Boost);
    fts.setRequestTtlMs(5000);
    FtsRequest r;
    r.primary_role = Role::RangeSafety;
    r.secondary_role = Role::SafetyOfficer;
    r.primary_user = "a"; r.secondary_user = "b";
    r.timestamp_ms = 100;
    g = 10000;
    M130_REQUIRE_EQ(fts.armAndFire(r).result, FtsResult::RejectedExpired);
    return 0;
}

int rejectsBadTotp()
{
    FlightTerminationService fts(&alwaysNo, nullptr, &clk);
    fts.setPhase(FlightPhase::Boost);
    FtsRequest r;
    r.primary_role = Role::RangeSafety;
    r.secondary_role = Role::SafetyOfficer;
    r.primary_user = "a"; r.secondary_user = "b";
    g = 100; r.timestamp_ms = 100;
    M130_REQUIRE_EQ(fts.armAndFire(r).result, FtsResult::RejectedPrimaryAuth);
    return 0;
}

int happyPathDispatches()
{
    int dispatched = 0;
    FlightTerminationService fts(&alwaysYes, [&](const FtsRequest&){ ++dispatched; }, &clk);
    fts.setPhase(FlightPhase::Cruise);
    FtsRequest r;
    r.primary_role = Role::RangeSafety;
    r.secondary_role = Role::SafetyOfficer;
    r.primary_user = "rso"; r.secondary_user = "safety";
    r.primary_totp = "111111"; r.secondary_totp = "222222";
    g = 1000; r.timestamp_ms = 1000;
    auto d = fts.armAndFire(r);
    M130_REQUIRE_EQ(d.result, FtsResult::ArmedAndSent);
    M130_REQUIRE_EQ(dispatched, 1);
    return 0;
}

int run()
{
    M130_RUN(rejectsLowRoles);
    M130_RUN(rejectsSameUser);
    M130_RUN(rejectsOnGround);
    M130_RUN(rejectsExpired);
    M130_RUN(rejectsBadTotp);
    M130_RUN(happyPathDispatches);
    return 0;
}

M130_TEST_MAIN()

#include "safety/CommandAuthorization.h"
#include "test_support.h"

using namespace m130::safety;
using namespace m130::access;

namespace { uint64_t g = 0; uint64_t clk() { return g += 100; } }

int defaultDenyUnknown()
{
    CommandAuthorization a(&clk);
    AuthRequest r; r.command = "NO_SUCH_CMD"; r.granted_role = Role::Admin;
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::DeniedUnknown);
    return 0;
}

int rbacRoles()
{
    CommandAuthorization a(&clk);
    AuthRequest r; r.command = "M130_COMMAND_ARM"; r.phase = FlightPhase::Prelaunch;
    r.granted_role = Role::Observer;
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::DeniedRole);
    r.granted_role = Role::Operator;
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::Allowed);
    return 0;
}

int phaseGuard()
{
    CommandAuthorization a(&clk);
    AuthRequest r; r.command = "M130_COMMAND_ARM"; r.granted_role = Role::Operator;
    r.phase = FlightPhase::Boost;
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::DeniedPhase);
    return 0;
}

int dualAuthEnforced()
{
    CommandAuthorization a(&clk);
    AuthRequest r;
    r.command = "M130_COMMAND_FTS";
    r.phase = FlightPhase::Boost;
    r.granted_role = Role::RangeSafety;
    r.primary_user = "rso1";
    // No secondary
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::DeniedDualAuth);
    // Secondary present but same user
    r.second_role = Role::SafetyOfficer;
    r.second_user = std::string("rso1");
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::DeniedDualAuth);
    // Distinct users
    r.second_user = std::string("safety2");
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::Allowed);
    return 0;
}

int rangeCheck()
{
    CommandAuthorization a(&clk);
    AuthRequest r;
    r.command = "M130_COMMAND_CHECKLIST_SIGN";
    r.granted_role = Role::Operator;
    r.phase = FlightPhase::Prelaunch;
    r.args["result"] = 5.0; // out of [0,2]
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::DeniedRange);
    r.args["result"] = 1.0;
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::Allowed);
    return 0;
}

int rateLimit()
{
    CommandAuthorization a(&clk);
    AuthRequest r; r.command = "M130_COMMAND_ARM"; r.granted_role = Role::Operator; r.phase = FlightPhase::Prelaunch;
    // default max_per_minute = 6
    for (int i = 0; i < 6; ++i) {
        M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::Allowed);
    }
    M130_REQUIRE_EQ(a.evaluate(r).result, AuthResult::DeniedRate);
    return 0;
}

int run()
{
    M130_RUN(defaultDenyUnknown);
    M130_RUN(rbacRoles);
    M130_RUN(phaseGuard);
    M130_RUN(dualAuthEnforced);
    M130_RUN(rangeCheck);
    M130_RUN(rateLimit);
    return 0;
}

M130_TEST_MAIN()

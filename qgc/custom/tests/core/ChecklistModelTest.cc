#include "access/Role.h"
#include "test_support.h"
#include "views/ChecklistModel.h"

using namespace m130::views;
using m130::access::Role;

namespace {
std::uint64_t fakeClock() { return 12345; }
} // namespace

static int addAndFindItems()
{
    ChecklistModel cl;
    ChecklistItem a{"fuel.load", "Fuel load", "Load to 100%", false, Role::Operator};
    ChecklistItem b{"weather.go", "Weather go", "Winds < 10 m/s", true, Role::FlightDirector};
    M130_REQUIRE(cl.add(a));
    M130_REQUIRE(cl.add(b));
    M130_REQUIRE(!cl.add(a)); // duplicate rejected
    M130_REQUIRE_EQ(cl.size(), static_cast<std::size_t>(2));
    M130_REQUIRE(cl.find("fuel.load").has_value());
    M130_REQUIRE(!cl.find("missing").has_value());
    return 0;
}

static int markDoneRespectsAuth()
{
    ChecklistModel cl(&fakeClock);
    cl.add({"signoff", "Final sign-off", "", true, Role::FlightDirector});

    // No user — rejected.
    M130_REQUIRE(!cl.markDone("signoff", {}, Role::FlightDirector));
    // Insufficient role — rejected.
    M130_REQUIRE(!cl.markDone("signoff", "alice", Role::Operator));
    // Correct role — accepted.
    M130_REQUIRE(cl.markDone("signoff", "alice", Role::FlightDirector, "OK"));

    const auto it = cl.find("signoff").value();
    M130_REQUIRE_EQ(it.status, ChecklistStatus::Done);
    M130_REQUIRE_EQ(it.completed_by, std::string{"alice"});
    M130_REQUIRE_EQ(it.completed_at_ms, std::uint64_t{12345});
    M130_REQUIRE_EQ(it.notes, std::string{"OK"});
    return 0;
}

static int skipRequiresReason()
{
    ChecklistModel cl;
    cl.add({"cam.check", "Camera check", "", false, Role::None});
    M130_REQUIRE(!cl.skip("cam.check", "bob", {}));           // empty reason
    M130_REQUIRE(cl.skip("cam.check", "bob", "cam not fitted"));
    M130_REQUIRE_EQ(cl.find("cam.check")->status, ChecklistStatus::Skipped);
    return 0;
}

static int blockedItemRejectsDone()
{
    ChecklistModel cl;
    cl.add({"sensor", "Sensor cal", "", false, Role::None});
    M130_REQUIRE(cl.block("sensor", "bus fault"));
    M130_REQUIRE(!cl.markDone("sensor", "alice", Role::Operator));
    M130_REQUIRE(!cl.skip("sensor", "alice", "workaround"));
    return 0;
}

static int readyRequiresAllDone()
{
    ChecklistModel cl;
    cl.add({"a", "A", "", false, Role::None});
    cl.add({"b", "B", "", false, Role::None});
    M130_REQUIRE(!cl.isReadyForLaunch());
    cl.markDone("a", "u", Role::None);
    M130_REQUIRE(!cl.isReadyForLaunch());
    cl.skip("b", "u", "n/a");
    M130_REQUIRE(!cl.isReadyForLaunch()); // skipped counts as blocker
    cl.markDone("b", "u", Role::None);
    M130_REQUIRE(cl.isReadyForLaunch());
    return 0;
}

static int resetClearsStatus()
{
    ChecklistModel cl;
    cl.add({"x", "X", "", false, Role::None});
    cl.markDone("x", "u", Role::None, "done");
    cl.reset();
    M130_REQUIRE_EQ(cl.find("x")->status, ChecklistStatus::Pending);
    M130_REQUIRE(cl.find("x")->completed_by.empty());
    return 0;
}

static int run()
{
    M130_RUN(addAndFindItems);
    M130_RUN(markDoneRespectsAuth);
    M130_RUN(skipRequiresReason);
    M130_RUN(blockedItemRejectsDone);
    M130_RUN(readyRequiresAllDone);
    M130_RUN(resetClearsStatus);
    return 0;
}

M130_TEST_MAIN()

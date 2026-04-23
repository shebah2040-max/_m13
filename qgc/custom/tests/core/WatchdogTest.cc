#include "safety/Watchdog.h"
#include "test_support.h"

using namespace m130::safety;

namespace {
uint64_t g_time = 0;
uint64_t clock_() { return g_time; }
} // namespace

int freshChannelNoEmergencyBeforeUpdate()
{
    g_time = 0;
    Watchdog w(&clock_);
    w.addChannel("heartbeat");
    AlertLevel l = w.tick(FlightPhase::Idle);
    M130_REQUIRE_EQ(l, AlertLevel::Advisory); // never updated yet
    return 0;
}

int levelsScaleWithAge()
{
    g_time = 0;
    Watchdog w(&clock_);
    w.addChannel("heartbeat");
    g_time = 100;
    w.feed("heartbeat");

    // Within advisory: age 200 ms (500 ms threshold)
    g_time = 300;
    M130_REQUIRE_EQ(w.tick(FlightPhase::Idle), AlertLevel::None);

    // Past advisory (age 600 ms)
    g_time = 700;
    M130_REQUIRE_EQ(w.tick(FlightPhase::Idle), AlertLevel::Advisory);

    // Past caution (age 1200 ms)
    g_time = 1300;
    M130_REQUIRE_EQ(w.tick(FlightPhase::Idle), AlertLevel::Caution);

    // Past warning (age 4000 ms)
    g_time = 4100;
    M130_REQUIRE_EQ(w.tick(FlightPhase::Idle), AlertLevel::Warning);

    // Emergency only in flight
    g_time = 12000;
    M130_REQUIRE_EQ(w.tick(FlightPhase::Idle),  AlertLevel::Warning);
    M130_REQUIRE_EQ(w.tick(FlightPhase::Boost), AlertLevel::Emergency);
    return 0;
}

int subscribersReceiveEvents()
{
    g_time = 0;
    Watchdog w(&clock_);
    w.addChannel("hb");
    int events = 0;
    w.subscribe([&](const Watchdog::Event&) { ++events; });
    w.feed("hb");
    g_time = 5000;
    w.tick(FlightPhase::Boost);
    M130_EXPECT(events >= 1);
    return 0;
}

int unknownChannelFeedIsNoOp()
{
    g_time = 0;
    Watchdog w(&clock_);
    w.feed("nonexistent"); // should not crash
    (void)w.tick(FlightPhase::Idle);
    return 0;
}

int run()
{
    M130_RUN(freshChannelNoEmergencyBeforeUpdate);
    M130_RUN(levelsScaleWithAge);
    M130_RUN(subscribersReceiveEvents);
    M130_RUN(unknownChannelFeedIsNoOp);
    return 0;
}

M130_TEST_MAIN()

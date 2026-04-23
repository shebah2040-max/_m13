#include "test_support.h"
#include "views/CountdownClock.h"

#include <cmath>

using namespace m130::views;

static int armRejectsPastTarget()
{
    CountdownClock c;
    M130_REQUIRE(!c.arm(1000, 1000));
    M130_REQUIRE(!c.arm(500, 1000));
    M130_REQUIRE(c.arm(2000, 1000));
    M130_REQUIRE_EQ(c.state(), CountdownState::Counting);
    return 0;
}

static int countsDown()
{
    CountdownClock c;
    c.arm(10000, 0);
    M130_REQUIRE(std::fabs(c.secondsToLaunch(0) - 10.0) < 1e-9);
    M130_REQUIRE(std::fabs(c.secondsToLaunch(5000) - 5.0) < 1e-9);
    return 0;
}

static int holdFreezesAndResumeShiftsTarget()
{
    CountdownClock c;
    c.arm(10000, 0);
    c.tick(4000);
    M130_REQUIRE(c.hold(4000));
    M130_REQUIRE_EQ(c.state(), CountdownState::Hold);

    // Advance wall clock by 3s while holding — stays 6s to launch.
    const double freezeS = c.secondsToLaunch(7000);
    M130_REQUIRE(std::fabs(freezeS - 6.0) < 1e-9);

    M130_REQUIRE(c.resume(7000));
    // Target shifted by 3s to 13000; now is 7000 → 6s to launch.
    M130_REQUIRE(std::fabs(c.secondsToLaunch(7000) - 6.0) < 1e-9);
    M130_REQUIRE_EQ(c.holdElapsedMs(), std::uint64_t{3000});
    return 0;
}

static int tickTransitionsToLaunched()
{
    CountdownClock c;
    c.arm(1000, 0);
    c.tick(500);
    M130_REQUIRE_EQ(c.state(), CountdownState::Counting);
    c.tick(1500);
    M130_REQUIRE_EQ(c.state(), CountdownState::Launched);
    // Post-launch seconds are negative.
    M130_REQUIRE(c.secondsToLaunch(2000) < 0.0);
    return 0;
}

static int abortIsTerminal()
{
    CountdownClock c;
    c.arm(10000, 0);
    c.abort(5000);
    M130_REQUIRE_EQ(c.state(), CountdownState::Aborted);
    M130_REQUIRE(!c.hold(6000));
    M130_REQUIRE(!c.resume(6000));
    return 0;
}

static int labelFormatting()
{
    CountdownClock c;
    c.arm(1 * 60 * 60 * 1000 + 23 * 60 * 1000 + 45 * 1000, 0);
    const auto s = c.label(0);
    M130_REQUIRE_EQ(s, std::string{"T-01:23:45"});
    return 0;
}

static int run()
{
    M130_RUN(armRejectsPastTarget);
    M130_RUN(countsDown);
    M130_RUN(holdFreezesAndResumeShiftsTarget);
    M130_RUN(tickTransitionsToLaunched);
    M130_RUN(abortIsTerminal);
    M130_RUN(labelFormatting);
    return 0;
}

M130_TEST_MAIN()

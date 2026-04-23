#include "safety/MissionStateMachine.h"
#include "test_support.h"

using namespace m130::safety;

namespace {
uint64_t fakeClock()
{
    static uint64_t t = 0;
    return t += 10;
}
} // namespace

int legalHappyPath()
{
    MissionStateMachine sm(&fakeClock);
    M130_REQUIRE_EQ(sm.current(), FlightPhase::Unknown);
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Idle, "boot"),      TransitionResult::Accepted);
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Prelaunch, "prep"), TransitionResult::Accepted);
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Armed, "arm"),      TransitionResult::Accepted);
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Boost, "launch"),   TransitionResult::Accepted);
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Cruise, "burnout"), TransitionResult::Accepted);
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Terminal, "descent"), TransitionResult::Accepted);
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Landed, "touchdown"), TransitionResult::Accepted);
    return 0;
}

int rejectsIllegal()
{
    MissionStateMachine sm(&fakeClock);
    sm.requestTransition(FlightPhase::Idle);
    // Idle → Boost is not legal (must pass Prelaunch/Armed).
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Boost), TransitionResult::RejectedIllegal);
    M130_REQUIRE_EQ(sm.current(), FlightPhase::Idle);
    return 0;
}

int rejectsNoChange()
{
    MissionStateMachine sm(&fakeClock);
    sm.requestTransition(FlightPhase::Idle);
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Idle), TransitionResult::RejectedNoChange);
    return 0;
}

int landedIsTerminal()
{
    MissionStateMachine sm(&fakeClock);
    sm.requestTransition(FlightPhase::Idle);
    sm.requestTransition(FlightPhase::Prelaunch);
    sm.requestTransition(FlightPhase::Armed);
    sm.requestTransition(FlightPhase::Boost);
    sm.requestTransition(FlightPhase::Cruise);
    sm.requestTransition(FlightPhase::Terminal);
    sm.requestTransition(FlightPhase::Landed);
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Idle), TransitionResult::RejectedTerminal);
    M130_REQUIRE_EQ(sm.current(), FlightPhase::Landed);
    return 0;
}

int abortOverridesGuards()
{
    MissionStateMachine sm(&fakeClock);
    sm.requestTransition(FlightPhase::Idle);
    sm.requestTransition(FlightPhase::Prelaunch);
    sm.requestTransition(FlightPhase::Armed);
    sm.requestTransition(FlightPhase::Boost);
    // Guard that always rejects — but ABORT must override.
    sm.setGuard(FlightPhase::Boost, FlightPhase::Abort, [](FlightPhase, FlightPhase) { return false; });
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Abort, "RSO"), TransitionResult::Accepted);
    M130_REQUIRE_EQ(sm.current(), FlightPhase::Abort);
    return 0;
}

int guardCanBlockNormalTransition()
{
    MissionStateMachine sm(&fakeClock);
    sm.requestTransition(FlightPhase::Idle);
    sm.setGuard(FlightPhase::Idle, FlightPhase::Prelaunch, [](FlightPhase, FlightPhase) { return false; });
    M130_REQUIRE_EQ(sm.requestTransition(FlightPhase::Prelaunch, "no"), TransitionResult::RejectedGuard);
    M130_REQUIRE_EQ(sm.current(), FlightPhase::Idle);
    return 0;
}

int listenerReceivesEvents()
{
    MissionStateMachine sm(&fakeClock);
    int count = 0;
    sm.addListener([&](const TransitionRecord&) { ++count; });
    sm.requestTransition(FlightPhase::Idle);
    sm.requestTransition(FlightPhase::Prelaunch);
    M130_REQUIRE_EQ(count, 2);
    return 0;
}

int historyIsRecorded()
{
    MissionStateMachine sm(&fakeClock);
    sm.requestTransition(FlightPhase::Idle, "a");
    sm.requestTransition(FlightPhase::Boost, "illegal"); // rejected
    M130_REQUIRE_EQ(sm.history().size(), std::size_t(2));
    M130_REQUIRE_EQ(sm.history()[0].result, TransitionResult::Accepted);
    M130_REQUIRE_EQ(sm.history()[1].result, TransitionResult::RejectedIllegal);
    return 0;
}

int run()
{
    M130_RUN(legalHappyPath);
    M130_RUN(rejectsIllegal);
    M130_RUN(rejectsNoChange);
    M130_RUN(landedIsTerminal);
    M130_RUN(abortOverridesGuards);
    M130_RUN(guardCanBlockNormalTransition);
    M130_RUN(listenerReceivesEvents);
    M130_RUN(historyIsRecorded);
    return 0;
}

M130_TEST_MAIN()

#include "safety/SafetyAggregator.h"
#include "test_support.h"

#include <string>
#include <vector>

using namespace m130::safety;

namespace {

uint64_t g_now = 0;
uint64_t now() { return g_now; }

int envelopeViolationRaisesAlert()
{
    g_now = 0;
    SafetyAggregator a(&now);
    a.installDefaults();
    a.requestTransition(FlightPhase::Idle);

    std::vector<Alert> raised;
    a.subscribeAlerts([&](const Alert& al, bool is_ack) {
        if (!is_ack) raised.push_back(al);
    });

    // phi < -90 triggers Warning per default envelope.
    g_now = 10;
    const auto r = a.evaluateSample("phi", -120.0);
    M130_REQUIRE(r.level == AlertLevel::Warning);
    M130_REQUIRE(a.masterLevel() == AlertLevel::Warning);
    M130_REQUIRE_EQ(raised.size(), std::size_t(1));
    M130_REQUIRE(raised.front().id == "envelope.phi");
    M130_REQUIRE(raised.front().level == AlertLevel::Warning);

    // A subsequent nominal sample clears the alert.
    g_now = 20;
    const auto r2 = a.evaluateSample("phi", 0.0);
    M130_REQUIRE(r2.level == AlertLevel::None);
    M130_REQUIRE_EQ(a.activeAlertCount(), std::size_t(0));
    M130_REQUIRE(a.masterLevel() == AlertLevel::None);
    return 0;
}

int watchdogStalenessBecomesAlert()
{
    g_now = 0;
    SafetyAggregator a(&now);
    a.installDefaults();
    a.requestTransition(FlightPhase::Idle);
    a.addChannel("heartbeat");

    std::vector<Alert> raised;
    a.subscribeAlerts([&](const Alert& al, bool is_ack) {
        if (!is_ack) raised.push_back(al);
    });

    g_now = 100;
    a.feed("heartbeat");

    // 400 ms later — under all thresholds, no alert expected.
    g_now = 500;
    M130_REQUIRE(a.tick() == AlertLevel::None);
    M130_REQUIRE_EQ(raised.size(), std::size_t(0));

    // 2 s since last feed → Caution.
    g_now = 2100;
    M130_REQUIRE(a.tick() == AlertLevel::Caution);
    M130_REQUIRE(a.masterLevel() == AlertLevel::Caution);
    M130_REQUIRE(!raised.empty());
    M130_REQUIRE(raised.back().id == "stale.heartbeat");

    // Feeding fresh data clears the stale alert.
    g_now = 2200;
    a.feed("heartbeat");
    M130_REQUIRE(a.masterLevel() == AlertLevel::None);
    return 0;
}

int deniedCommandRaisesAdvisory()
{
    g_now = 0;
    SafetyAggregator a(&now);
    a.installDefaults();
    a.requestTransition(FlightPhase::Idle);
    a.requestTransition(FlightPhase::Prelaunch);

    std::vector<Alert> raised;
    a.subscribeAlerts([&](const Alert& al, bool is_ack) {
        if (!is_ack) raised.push_back(al);
    });

    // An Observer cannot issue M130_COMMAND_ARM; phase will be filled from
    // aggregator state (currently Prelaunch).
    AuthRequest r;
    r.command = "M130_COMMAND_ARM";
    r.granted_role = m130::access::Role::Observer;
    r.primary_user = "obs1";
    const AuthDecision d = a.evaluateCommand(r);
    M130_REQUIRE(d.result == AuthResult::DeniedRole);
    M130_REQUIRE_EQ(raised.size(), std::size_t(1));
    M130_REQUIRE(raised.back().level == AlertLevel::Advisory);
    M130_REQUIRE(raised.back().id == "cmd.denied.M130_COMMAND_ARM");
    return 0;
}

int rejectedTransitionRaisesCaution()
{
    g_now = 0;
    SafetyAggregator a(&now);
    a.installDefaults();
    a.requestTransition(FlightPhase::Idle);

    std::vector<Alert> raised;
    a.subscribeAlerts([&](const Alert& al, bool is_ack) {
        if (!is_ack) raised.push_back(al);
    });

    // Idle → Boost is illegal.
    const TransitionResult r = a.requestTransition(FlightPhase::Boost, "operator error");
    M130_REQUIRE(r == TransitionResult::RejectedIllegal);
    M130_REQUIRE_EQ(raised.size(), std::size_t(1));
    M130_REQUIRE(raised.back().id == "mission.transition");
    M130_REQUIRE(raised.back().level == AlertLevel::Caution);

    // A legal transition clears the caution.
    M130_REQUIRE(a.requestTransition(FlightPhase::Prelaunch) == TransitionResult::Accepted);
    M130_REQUIRE_EQ(a.activeAlertCount(), std::size_t(0));
    return 0;
}

int emergencyStalenessOnlyInFlight()
{
    g_now = 0;
    SafetyAggregator a(&now);
    a.installDefaults();
    a.requestTransition(FlightPhase::Idle);
    a.addChannel("heartbeat");

    g_now = 10;
    a.feed("heartbeat");

    // 20 s later on the ground → still Warning, not Emergency.
    g_now = 20010;
    M130_REQUIRE(a.tick() == AlertLevel::Warning);

    // Transition to in-flight and re-tick — now Emergency (ABORT requires
    // going through Armed → Boost, which requires feeding heartbeat first).
    g_now = 20020;
    a.feed("heartbeat");
    a.requestTransition(FlightPhase::Prelaunch);
    a.requestTransition(FlightPhase::Armed);
    a.requestTransition(FlightPhase::Boost);
    g_now = 40020;
    M130_REQUIRE(a.tick() == AlertLevel::Emergency);
    M130_REQUIRE(a.masterLevel() == AlertLevel::Emergency);
    return 0;
}

int run()
{
    M130_RUN(envelopeViolationRaisesAlert);
    M130_RUN(watchdogStalenessBecomesAlert);
    M130_RUN(deniedCommandRaisesAdvisory);
    M130_RUN(rejectedTransitionRaisesCaution);
    M130_RUN(emergencyStalenessOnlyInFlight);
    return 0;
}

} // namespace

M130_TEST_MAIN()

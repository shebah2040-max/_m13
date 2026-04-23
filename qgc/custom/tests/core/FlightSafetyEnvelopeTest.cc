#include "safety/FlightSafetyEnvelope.h"
#include "test_support.h"

using namespace m130::safety;

int classifyLevels()
{
    EnvelopeBounds b;
    b.low_emergency = -100; b.low_warning = -50; b.low_caution = -20; b.low_advisory = -10;
    b.high_advisory = 10;   b.high_caution = 20;  b.high_warning = 50;  b.high_emergency = 100;

    M130_REQUIRE_EQ(FlightSafetyEnvelope::classify(0,    b), AlertLevel::None);
    M130_REQUIRE_EQ(FlightSafetyEnvelope::classify(15,   b), AlertLevel::Advisory);
    M130_REQUIRE_EQ(FlightSafetyEnvelope::classify(30,   b), AlertLevel::Caution);
    M130_REQUIRE_EQ(FlightSafetyEnvelope::classify(70,   b), AlertLevel::Warning);
    M130_REQUIRE_EQ(FlightSafetyEnvelope::classify(999,  b), AlertLevel::Emergency);
    M130_REQUIRE_EQ(FlightSafetyEnvelope::classify(-70,  b), AlertLevel::Warning);
    M130_REQUIRE_EQ(FlightSafetyEnvelope::classify(-999, b), AlertLevel::Emergency);
    return 0;
}

int defaultEnvelopeLoaded()
{
    auto e = FlightSafetyEnvelope::createDefault();
    M130_REQUIRE(e.envelopes().count("phi"));
    M130_REQUIRE(e.envelopes().count("alpha_est"));
    M130_REQUIRE(e.envelopes().count("mpc_solve_us"));
    auto r = e.check("phi", 0.0, FlightPhase::Boost);
    M130_REQUIRE_EQ(r.level, AlertLevel::None);
    auto r2 = e.check("phi", 120.0, FlightPhase::Boost);
    M130_REQUIRE_EQ(r2.level, AlertLevel::Warning);
    return 0;
}

int unknownVariableNoAlert()
{
    auto e = FlightSafetyEnvelope::createDefault();
    auto r = e.check("not_monitored", 1e9, FlightPhase::Cruise);
    M130_REQUIRE_EQ(r.level, AlertLevel::None);
    return 0;
}

int subscriberReceivesResult()
{
    auto e = FlightSafetyEnvelope::createDefault();
    int count = 0;
    e.subscribe([&](const EnvelopeCheckResult&) { ++count; });
    e.check("phi", 0.0, FlightPhase::Boost);
    e.check("phi", 120.0, FlightPhase::Boost);
    M130_REQUIRE_EQ(count, 2);
    return 0;
}

int run()
{
    M130_RUN(classifyLevels);
    M130_RUN(defaultEnvelopeLoaded);
    M130_RUN(unknownVariableNoAlert);
    M130_RUN(subscriberReceivesResult);
    return 0;
}

M130_TEST_MAIN()

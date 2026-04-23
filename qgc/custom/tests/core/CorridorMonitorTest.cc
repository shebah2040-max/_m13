#include "nav/CorridorMonitor.h"
#include "nav/MapCorridor.h"
#include "test_support.h"

#include <chrono>

using namespace m130::nav;
using namespace m130::geo;
using namespace std::chrono_literals;

namespace {
MapCorridor squareCorridor()
{
    MapCorridor c;
    c.setPolygon({
        {34.5, 44.5, 0.0}, {34.5, 45.5, 0.0},
        {35.5, 45.5, 0.0}, {35.5, 44.5, 0.0},
    });
    return c;
}
}

int unknownWithoutCorridor()
{
    CorridorMonitor m;
    auto now = std::chrono::steady_clock::time_point{};
    auto s = m.update({35.0, 45.0, 0.0}, now);
    M130_REQUIRE(s.state == CorridorState::Unknown);
    return 0;
}

int insideReportedWhenComfortable()
{
    auto c = squareCorridor();
    CorridorMonitor m; m.setCorridor(&c);
    CorridorConfig cfg; cfg.warn_margin_m = 500.0; cfg.clear_margin_m = 600.0;
    m.setConfig(cfg);
    auto now = std::chrono::steady_clock::time_point{};
    auto s = m.update({35.0, 45.0, 0.0}, now);
    M130_REQUIRE(s.state == CorridorState::Inside);
    return 0;
}

int breachRequiresPersistence()
{
    auto c = squareCorridor();
    CorridorMonitor m; m.setCorridor(&c);
    CorridorConfig cfg;
    cfg.breach_persistence = 200ms;
    m.setConfig(cfg);
    auto t = std::chrono::steady_clock::time_point{};

    // First outside sample → not yet Breach.
    auto s1 = m.update({40.0, 45.0, 0.0}, t);
    M130_REQUIRE(s1.state != CorridorState::Breach);

    // Still outside but < persistence window.
    auto s2 = m.update({40.0, 45.0, 0.0}, t + 50ms);
    M130_REQUIRE(s2.state != CorridorState::Breach);

    // Past persistence window → Breach.
    auto s3 = m.update({40.0, 45.0, 0.0}, t + 300ms);
    M130_REQUIRE(s3.state == CorridorState::Breach);
    M130_REQUIRE(s3.just_breached);
    return 0;
}

int clearedWithHysteresis()
{
    auto c = squareCorridor();
    CorridorMonitor m; m.setCorridor(&c);
    CorridorConfig cfg;
    cfg.warn_margin_m  = 500.0;
    cfg.clear_margin_m = 50000.0;
    cfg.breach_persistence = 0ms;
    m.setConfig(cfg);

    auto t = std::chrono::steady_clock::time_point{};
    // Breach first.
    m.update({40.0, 45.0, 0.0}, t);
    M130_REQUIRE(m.state() == CorridorState::Breach);

    // Crossing back with margin < clear_margin_m → stays Warning.
    auto s = m.update({35.49, 45.0, 0.0}, t + 10ms);
    M130_REQUIRE(s.state == CorridorState::Warning);

    // Well inside → Inside + just_cleared.
    auto s2 = m.update({35.0, 45.0, 0.0}, t + 20ms);
    M130_REQUIRE(s2.state == CorridorState::Inside);
    M130_REQUIRE(s2.just_cleared);
    return 0;
}

int run()
{
    M130_RUN(unknownWithoutCorridor);
    M130_RUN(insideReportedWhenComfortable);
    M130_RUN(breachRequiresPersistence);
    M130_RUN(clearedWithHysteresis);
    return 0;
}

M130_TEST_MAIN()

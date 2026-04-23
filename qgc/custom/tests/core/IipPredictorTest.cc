#include "test_support.h"
#include "views/IipPredictor.h"

#include <cmath>

using namespace m130::views;

static int fallsStraightDown()
{
    // 100 m altitude, zero velocity — time = sqrt(2 h / g).
    const auto r = IipPredictor::predict(0, 0, 100, 0, 0, 0);
    M130_REQUIRE(r.valid);
    const double expected_t = std::sqrt(2.0 * 100.0 / IipPredictor::kGravity);
    M130_REQUIRE(std::fabs(r.time_to_impact_s - expected_t) < 1e-6);
    M130_REQUIRE(std::fabs(r.impact_downrange_m)  < 1e-9);
    M130_REQUIRE(std::fabs(r.impact_crossrange_m) < 1e-9);
    return 0;
}

static int risesThenFalls()
{
    // vz = +20 m/s at h=0. Time to apex = 20/g; then falls back down.
    // Total impact time = 2 * 20 / g.
    const auto r = IipPredictor::predict(0, 0, 0, 100, 10, 20);
    M130_REQUIRE(r.valid);
    const double expected_t = 2.0 * 20.0 / IipPredictor::kGravity;
    M130_REQUIRE(std::fabs(r.time_to_impact_s - expected_t) < 1e-6);
    M130_REQUIRE(std::fabs(r.impact_downrange_m  - 100.0 * expected_t) < 1e-3);
    M130_REQUIRE(std::fabs(r.impact_crossrange_m - 10.0  * expected_t) < 1e-3);
    return 0;
}

static int rejectsNegativeAltitudeWithUpwardVel()
{
    // Starting below ground with downward velocity — disc < 0 only if vz small.
    // Specifically (-1 m AGL, vz = 0) → disc = -2g < 0 → invalid.
    const auto r = IipPredictor::predict(0, 0, -1, 0, 0, 0);
    M130_REQUIRE(!r.valid);
    return 0;
}

static int rejectsZeroGravity()
{
    const auto r = IipPredictor::predict(0, 0, 100, 0, 0, 0, 0.0);
    M130_REQUIRE(!r.valid);
    return 0;
}

static int run()
{
    M130_RUN(fallsStraightDown);
    M130_RUN(risesThenFalls);
    M130_RUN(rejectsNegativeAltitudeWithUpwardVel);
    M130_RUN(rejectsZeroGravity);
    return 0;
}

M130_TEST_MAIN()

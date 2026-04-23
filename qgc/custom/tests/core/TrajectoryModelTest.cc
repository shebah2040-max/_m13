#include "nav/TrajectoryModel.h"
#include "test_support.h"

#include <chrono>
#include <cmath>

using namespace m130::nav;
using namespace m130::geo;
using namespace std::chrono_literals;

namespace {
GeoPoint pad() { return GeoPoint{35.0, 45.0, 0.0}; }
}

int acceptsMonotonicSamples()
{
    TrajectoryModel m;
    m.setOrigin(pad());
    const auto t0 = std::chrono::system_clock::time_point{};
    TrajectorySample a{t0 + 0s, pad(),            0.0, 0.0, 10.0};
    TrajectorySample b{t0 + 1s, {35.001, 45.001, 100.0}, 1.0, 1.0, 20.0};
    M130_REQUIRE(m.append(a));
    M130_REQUIRE(m.append(b));
    M130_REQUIRE(m.samples().size() == 2);
    return 0;
}

int rejectsOutOfOrderSamples()
{
    TrajectoryModel m;
    m.setOrigin(pad());
    const auto t0 = std::chrono::system_clock::time_point{};
    M130_REQUIRE(m.append({t0 + 2s, pad(), 0, 0, 0}));
    M130_REQUIRE(!m.append({t0 + 1s, pad(), 0, 0, 0}));
    M130_REQUIRE(m.samples().size() == 1);
    return 0;
}

int capacityDropsOldest()
{
    TrajectoryModel m;
    m.setOrigin(pad());
    m.setCapacity(3);
    const auto t0 = std::chrono::system_clock::time_point{};
    for (int i = 0; i < 10; ++i) {
        m.append({t0 + std::chrono::seconds(i), pad(), 0, 0, 0});
    }
    M130_REQUIRE(m.samples().size() == 3);
    return 0;
}

int boundsTrackMovement()
{
    TrajectoryModel m;
    m.setOrigin(pad());
    const auto t0 = std::chrono::system_clock::time_point{};
    m.append({t0, pad(), 0, 0, 0});
    m.append({t0 + 1s, GeoPoint{35.0, 45.0, 1000.0}, 0, 0, 0});
    M130_REQUIRE(m.bounds().valid);
    M130_REQUIRE(m.bounds().max_up_m - m.bounds().min_up_m > 999.0);
    return 0;
}

int downsampleReturnsEndpoints()
{
    TrajectoryModel m;
    m.setOrigin(pad());
    const auto t0 = std::chrono::system_clock::time_point{};
    for (int i = 0; i < 100; ++i) {
        GeoPoint p{35.0, 45.0 + 0.0001 * i, 0.0};
        m.append({t0 + std::chrono::seconds(i), p, 0, 0, 0});
    }
    auto ds = m.trackEnu(10);
    M130_REQUIRE(ds.size() <= 12);
    M130_REQUIRE(ds.size() >= 2);
    return 0;
}

int iipsTrackedSeparately()
{
    TrajectoryModel m;
    m.setOrigin(pad());
    const auto t0 = std::chrono::system_clock::time_point{};
    m.appendIip(t0,        {35.01, 45.01, 0});
    m.appendIip(t0 + 1s,   {35.02, 45.02, 0});
    M130_REQUIRE(m.iips().size() == 2);
    M130_REQUIRE(m.samples().empty());
    M130_REQUIRE(m.bounds().valid);
    return 0;
}

int run()
{
    M130_RUN(acceptsMonotonicSamples);
    M130_RUN(rejectsOutOfOrderSamples);
    M130_RUN(capacityDropsOldest);
    M130_RUN(boundsTrackMovement);
    M130_RUN(downsampleReturnsEndpoints);
    M130_RUN(iipsTrackedSeparately);
    return 0;
}

M130_TEST_MAIN()

#include "geo/Geodesic.h"
#include "test_support.h"

#include <cmath>

using namespace m130::geo;

namespace {
constexpr double kPi = 3.141592653589793238462643383279502884;
bool close(double a, double b, double eps) { return std::fabs(a - b) <= eps; }
}

int haversineEquatorialDegree()
{
    // One degree of great-circle arc ≈ 111 195 m at mean radius.
    const GeoPoint a{0.0, 0.0, 0.0};
    const GeoPoint b{0.0, 1.0, 0.0};
    const double d = haversineDistance(a, b);
    M130_REQUIRE(d > 111000.0 && d < 111400.0);
    return 0;
}

int bearingDueNorth()
{
    const GeoPoint a{45.0, 10.0, 0.0};
    const GeoPoint b{46.0, 10.0, 0.0};
    const double brg = initialBearing(a, b);
    M130_REQUIRE(close(brg, 0.0, 1e-6));
    return 0;
}

int bearingDueEast()
{
    const GeoPoint a{0.0,  0.0, 0.0};
    const GeoPoint b{0.0, 10.0, 0.0};
    const double brg = initialBearing(a, b);
    M130_REQUIRE(close(brg, kPi / 2.0, 1e-6));
    return 0;
}

int destinationRoundtrip()
{
    const GeoPoint a{35.0, 45.0, 0.0};
    const GeoPoint b = destination(a, kPi / 4.0, 50000.0); // NE 50 km
    const double dist = haversineDistance(a, b);
    M130_REQUIRE(close(dist, 50000.0, 1.0));
    const double brg = initialBearing(a, b);
    M130_REQUIRE(close(brg, kPi / 4.0, 1e-3));
    return 0;
}

int crossTrackZeroOnLine()
{
    const GeoPoint a{0.0, 0.0, 0.0};
    const GeoPoint b{0.0, 10.0, 0.0};
    const GeoPoint p{0.0, 5.0, 0.0};
    const double xt = crossTrackDistance(a, b, p);
    M130_REQUIRE(std::fabs(xt) < 1.0);
    return 0;
}

int crossTrackOffsetNorth()
{
    // Track runs due east along the equator; p is due north of the
    // midpoint → cross-track distance ≈ 111 195 m.
    const GeoPoint a{0.0,  0.0, 0.0};
    const GeoPoint b{0.0, 10.0, 0.0};
    const GeoPoint p{1.0,  5.0, 0.0};
    const double xt = std::fabs(crossTrackDistance(a, b, p));
    M130_REQUIRE(xt > 110000.0 && xt < 112000.0);
    return 0;
}

int run()
{
    M130_RUN(haversineEquatorialDegree);
    M130_RUN(bearingDueNorth);
    M130_RUN(bearingDueEast);
    M130_RUN(destinationRoundtrip);
    M130_RUN(crossTrackZeroOnLine);
    M130_RUN(crossTrackOffsetNorth);
    return 0;
}

M130_TEST_MAIN()

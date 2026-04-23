#include "geo/Wgs84.h"
#include "test_support.h"

#include <cmath>

using namespace m130::geo;

namespace {
bool close(double a, double b, double eps) { return std::fabs(a - b) <= eps; }
}

int llaToEcefEquator()
{
    // On the equator at 0 longitude, X ≈ a + h, Y = Z = 0.
    EcefPoint e = llaToEcef(GeoPoint{0.0, 0.0, 0.0});
    M130_REQUIRE(close(e.x, wgs84::kA, 1e-6));
    M130_REQUIRE(close(e.y, 0.0, 1e-6));
    M130_REQUIRE(close(e.z, 0.0, 1e-6));
    return 0;
}

int llaToEcefNorthPole()
{
    EcefPoint e = llaToEcef(GeoPoint{90.0, 0.0, 0.0});
    M130_REQUIRE(close(e.x, 0.0, 1e-6));
    M130_REQUIRE(close(e.y, 0.0, 1e-6));
    // z at the pole = b (semi-minor axis).
    M130_REQUIRE(close(e.z, wgs84::kB, 1e-3));
    return 0;
}

int roundtripLlaEcefLla()
{
    const GeoPoint in{37.427619, -122.170322, 42.5}; // Palo Alto
    const EcefPoint e = llaToEcef(in);
    const GeoPoint  out = ecefToLla(e);
    M130_REQUIRE(close(out.lat_deg, in.lat_deg, 1e-8));
    M130_REQUIRE(close(out.lon_deg, in.lon_deg, 1e-8));
    M130_REQUIRE(close(out.alt_m,   in.alt_m,   1e-4));
    return 0;
}

int enuZeroAtOrigin()
{
    const GeoPoint origin{35.0, 45.0, 100.0};
    const EnuOffset o = llaToEnu(origin, origin);
    M130_REQUIRE(close(o.east_m,  0.0, 1e-6));
    M130_REQUIRE(close(o.north_m, 0.0, 1e-6));
    M130_REQUIRE(close(o.up_m,    0.0, 1e-6));
    return 0;
}

int enuNorthShift()
{
    // 1° of latitude north ≈ 111 km north.
    const GeoPoint origin{35.0, 45.0, 0.0};
    const GeoPoint north{36.0, 45.0, 0.0};
    const EnuOffset o = llaToEnu(north, origin);
    M130_REQUIRE(close(o.east_m, 0.0, 1.0));
    M130_REQUIRE(o.north_m > 110000.0 && o.north_m < 112000.0);
    M130_REQUIRE(std::fabs(o.up_m) < 1000.0);
    return 0;
}

int enuRoundtrip()
{
    const GeoPoint origin{35.0, 45.0, 100.0};
    const GeoPoint p{35.01, 45.02, 250.0};
    const EnuOffset o = llaToEnu(p, origin);
    const GeoPoint back = enuToLla(o, origin);
    M130_REQUIRE(close(back.lat_deg, p.lat_deg, 1e-8));
    M130_REQUIRE(close(back.lon_deg, p.lon_deg, 1e-8));
    M130_REQUIRE(close(back.alt_m,   p.alt_m,   1e-4));
    return 0;
}

int run()
{
    M130_RUN(llaToEcefEquator);
    M130_RUN(llaToEcefNorthPole);
    M130_RUN(roundtripLlaEcefLla);
    M130_RUN(enuZeroAtOrigin);
    M130_RUN(enuNorthShift);
    M130_RUN(enuRoundtrip);
    return 0;
}

M130_TEST_MAIN()

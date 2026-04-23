#include "nav/MapCorridor.h"
#include "test_support.h"

using namespace m130::nav;
using namespace m130::geo;

namespace {
// ~100 km square centred at (35, 45).
MapCorridor squareCorridor()
{
    MapCorridor c;
    std::vector<GeoPoint> poly = {
        {34.5, 44.5, 0.0},
        {34.5, 45.5, 0.0},
        {35.5, 45.5, 0.0},
        {35.5, 44.5, 0.0},
    };
    c.setPolygon(std::move(poly));
    return c;
}
}

int polygonContainsInteriorPoint()
{
    auto c = squareCorridor();
    M130_REQUIRE(c.contains({35.0, 45.0, 0.0}));
    return 0;
}

int polygonRejectsExteriorPoint()
{
    auto c = squareCorridor();
    M130_REQUIRE(!c.contains({40.0, 45.0, 0.0}));
    return 0;
}

int polygonReportsPositiveMarginInside()
{
    auto c = squareCorridor();
    auto m = c.marginMeters({35.0, 45.0, 0.0});
    M130_REQUIRE(m.has_value());
    M130_REQUIRE(*m > 10000.0);
    return 0;
}

int polygonReportsNegativeMarginOutside()
{
    auto c = squareCorridor();
    auto m = c.marginMeters({40.0, 45.0, 0.0});
    M130_REQUIRE(m.has_value());
    M130_REQUIRE(*m < 0.0);
    return 0;
}

int trackLegContainsNearCentreline()
{
    MapCorridor c;
    c.addTrackLeg({35.0, 45.0, 0.0}, {35.0, 45.1, 0.0}, 2000.0);
    M130_REQUIRE(c.contains({35.0, 45.05, 0.0}));
    return 0;
}

int trackLegRejectsBeyondHalfWidth()
{
    MapCorridor c;
    c.addTrackLeg({35.0, 45.0, 0.0}, {35.0, 45.1, 0.0}, 2000.0);
    // ~5.5 km north perpendicular offset.
    M130_REQUIRE(!c.contains({35.05, 45.05, 0.0}));
    return 0;
}

int emptyCorridorNeverContains()
{
    MapCorridor c;
    M130_REQUIRE(!c.contains({0.0, 0.0, 0.0}));
    auto m = c.marginMeters({0.0, 0.0, 0.0});
    M130_REQUIRE(!m.has_value());
    return 0;
}

int run()
{
    M130_RUN(polygonContainsInteriorPoint);
    M130_RUN(polygonRejectsExteriorPoint);
    M130_RUN(polygonReportsPositiveMarginInside);
    M130_RUN(polygonReportsNegativeMarginOutside);
    M130_RUN(trackLegContainsNearCentreline);
    M130_RUN(trackLegRejectsBeyondHalfWidth);
    M130_RUN(emptyCorridorNeverContains);
    return 0;
}

M130_TEST_MAIN()

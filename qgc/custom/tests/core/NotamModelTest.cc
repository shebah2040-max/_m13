#include "weather/NotamModel.h"
#include "test_support.h"

#include <chrono>

using namespace m130::weather;
using namespace m130::geo;
using namespace std::chrono_literals;

namespace {
using tp = std::chrono::system_clock::time_point;
tp at(long long sec) { return tp{} + std::chrono::seconds(sec); }
}

int circleContainsAndExcludes()
{
    NotamArea a;
    a.centre = {35.0, 45.0, 0.0};
    a.radius_m = 5000.0;
    M130_REQUIRE(a.isCircle());
    M130_REQUIRE(a.contains({35.0, 45.0, 0.0}));
    M130_REQUIRE(!a.contains({36.0, 45.0, 0.0}));
    return 0;
}

int polygonContainsAndExcludes()
{
    NotamArea a;
    a.polygon = {
        {34.9, 44.9, 0.0}, {34.9, 45.1, 0.0},
        {35.1, 45.1, 0.0}, {35.1, 44.9, 0.0},
    };
    M130_REQUIRE(a.isPolygon());
    M130_REQUIRE(a.contains({35.0, 45.0, 0.0}));
    M130_REQUIRE(!a.contains({36.0, 45.0, 0.0}));
    return 0;
}

int timeWindowFiltersActive()
{
    NotamModel m;
    Notam n;
    n.id = "A001";
    n.severity = NotamSeverity::Warning;
    n.start = at(100);
    n.end   = at(200);
    n.area.centre = {35.0, 45.0, 0.0};
    n.area.radius_m = 1000.0;
    m.add(n);

    M130_REQUIRE(m.activeAt(at(50)).empty());
    M130_REQUIRE(m.activeAt(at(150)).size() == 1);
    M130_REQUIRE(m.activeAt(at(250)).empty());
    return 0;
}

int worstSeverityChoosesMax()
{
    NotamModel m;
    Notam a; a.severity = NotamSeverity::Advisory;
    a.start = at(0); a.end = at(1000);
    a.area.centre = {35.0, 45.0, 0.0}; a.area.radius_m = 1000.0;
    m.add(a);

    Notam b; b.severity = NotamSeverity::Hazard;
    b.start = at(0); b.end = at(1000);
    b.area.centre = {35.0, 45.0, 0.0}; b.area.radius_m = 2000.0;
    m.add(b);

    Notam c; c.severity = NotamSeverity::Warning;
    c.start = at(0); c.end = at(1000);
    c.area.centre = {0.0, 0.0, 0.0}; c.area.radius_m = 1000.0;
    m.add(c);

    M130_REQUIRE(m.worstAt({35.0, 45.0, 0.0}, at(500)) == NotamSeverity::Hazard);
    M130_REQUIRE(m.worstAt({0.0, 0.0, 0.0},   at(500)) == NotamSeverity::Warning);
    M130_REQUIRE(m.worstAt({10.0, 10.0, 0.0}, at(500)) == NotamSeverity::Info);
    return 0;
}

int isAffectedMatchesAreaAndTime()
{
    NotamModel m;
    Notam n;
    n.severity = NotamSeverity::Warning;
    n.start = at(0); n.end = at(100);
    n.area.centre = {35.0, 45.0, 0.0};
    n.area.radius_m = 5000.0;
    m.add(n);

    M130_REQUIRE(m.isAffected({35.0, 45.0, 0.0}, at(50)));
    M130_REQUIRE(!m.isAffected({35.0, 45.0, 0.0}, at(200)));
    M130_REQUIRE(!m.isAffected({40.0, 45.0, 0.0}, at(50)));
    return 0;
}

int run()
{
    M130_RUN(circleContainsAndExcludes);
    M130_RUN(polygonContainsAndExcludes);
    M130_RUN(timeWindowFiltersActive);
    M130_RUN(worstSeverityChoosesMax);
    M130_RUN(isAffectedMatchesAreaAndTime);
    return 0;
}

M130_TEST_MAIN()

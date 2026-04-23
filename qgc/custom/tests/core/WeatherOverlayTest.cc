#include "weather/WeatherOverlay.h"
#include "test_support.h"

#include <cmath>

using namespace m130::weather;

namespace {
bool close(double a, double b, double eps) { return std::fabs(a - b) <= eps; }
}

int samplesInvalidWhenEmpty()
{
    WeatherOverlay w;
    auto s = w.sampleAt(35.0, 45.0);
    M130_REQUIRE(!s.valid);
    return 0;
}

int rejectsUndersizedGrid()
{
    WeatherOverlay w;
    std::vector<WeatherCell> cells(3, WeatherCell{});
    M130_REQUIRE(!w.setCells(1, 3, cells));
    M130_REQUIRE(!w.setCells(3, 1, cells));
    return 0;
}

int bilinearAtCornerMatchesCell()
{
    WeatherOverlay w;
    w.setBounds(0.0, 0.0, 1.0, 1.0);
    std::vector<WeatherCell> cells = {
        {  0.0, 10.0, 101000.0, 288.0 }, // row0 col0
        { 90.0, 20.0, 102000.0, 289.0 }, // row0 col1
        {180.0, 30.0, 103000.0, 290.0 }, // row1 col0
        {270.0, 40.0, 104000.0, 291.0 }, // row1 col1
    };
    M130_REQUIRE(w.setCells(2, 2, cells));

    // Lower-left corner (min_lat, min_lon) = row 0, col 0.
    auto a = w.sampleAt(0.0, 0.0);
    M130_REQUIRE(a.valid);
    M130_REQUIRE(close(a.pressure_pa,  101000.0, 1e-6));
    M130_REQUIRE(close(a.temperature_k, 288.0,   1e-6));

    // Upper-right corner = row 1, col 1.
    auto d = w.sampleAt(1.0, 1.0);
    M130_REQUIRE(d.valid);
    M130_REQUIRE(close(d.pressure_pa, 104000.0, 1e-6));
    return 0;
}

int bilinearInteriorIsAverage()
{
    WeatherOverlay w;
    w.setBounds(0.0, 0.0, 1.0, 1.0);
    std::vector<WeatherCell> cells(4);
    for (auto& c : cells) { c.pressure_pa = 100000.0; c.temperature_k = 288.0; }
    cells[3].pressure_pa = 108000.0;
    w.setCells(2, 2, cells);

    auto mid = w.sampleAt(0.5, 0.5);
    M130_REQUIRE(mid.valid);
    // Average of (100000 × 3 + 108000) / 4 = 102000.
    M130_REQUIRE(close(mid.pressure_pa, 102000.0, 1e-6));
    return 0;
}

int clampsOutsideBounds()
{
    WeatherOverlay w;
    w.setBounds(0.0, 0.0, 1.0, 1.0);
    std::vector<WeatherCell> cells = {
        { 0.0,  1.0, 100000.0, 288.0 },
        { 0.0,  2.0, 100000.0, 288.0 },
        { 0.0,  3.0, 100000.0, 288.0 },
        { 0.0,  4.0, 100000.0, 288.0 },
    };
    w.setCells(2, 2, cells);
    auto s = w.sampleAt(99.0, 99.0);  // clamp to UR corner cell
    M130_REQUIRE(s.valid);
    M130_REQUIRE(close(s.pressure_pa, 100000.0, 1e-6));
    return 0;
}

int run()
{
    M130_RUN(samplesInvalidWhenEmpty);
    M130_RUN(rejectsUndersizedGrid);
    M130_RUN(bilinearAtCornerMatchesCell);
    M130_RUN(bilinearInteriorIsAverage);
    M130_RUN(clampsOutsideBounds);
    return 0;
}

M130_TEST_MAIN()

#include "analysis/Timeseries.h"
#include "test_support.h"

#include <cmath>

using m130::analysis::Timeseries;

static int testBasicPushStats()
{
    Timeseries ts("roll_rate_radps", 8);
    ts.push(100, 1.0);
    ts.push(200, 2.0);
    ts.push(300, 3.0);
    ts.push(400, 4.0);
    M130_REQUIRE_EQ(ts.size(), std::size_t{4});
    M130_REQUIRE(std::abs(ts.mean() - 2.5) < 1e-9);
    M130_REQUIRE(std::abs(ts.minVal() - 1.0) < 1e-9);
    M130_REQUIRE(std::abs(ts.maxVal() - 4.0) < 1e-9);
    M130_REQUIRE(ts.stddev() > 1.0);
    M130_REQUIRE_EQ(ts.oldest().t_ms, std::uint64_t{100});
    M130_REQUIRE_EQ(ts.newest().t_ms, std::uint64_t{400});
    return 0;
}

static int testNonMonotonicClamp()
{
    Timeseries ts("q", 4);
    ts.push(500, 1.0);
    ts.push(400, 2.0);  // goes backwards — clamp to 500
    M130_REQUIRE_EQ(ts.at(1).t_ms, std::uint64_t{500});
    M130_REQUIRE(ts.at(1).value == 2.0);
    return 0;
}

static int testDownsample()
{
    Timeseries ts("x", 16);
    for (int i = 0; i < 16; ++i) {
        ts.push(static_cast<std::uint64_t>(i) * 100, static_cast<double>(i));
    }
    auto ds = ts.downsample(4);
    M130_REQUIRE_EQ(ds.size(), std::size_t{4});
    M130_REQUIRE(ds.front().t_ms == 0);
    M130_REQUIRE(ds.back().t_ms  == 1500);

    auto empty = Timeseries("e", 4).downsample(10);
    M130_REQUIRE(empty.empty());

    auto full = ts.downsample(100);
    M130_REQUIRE_EQ(full.size(), std::size_t{16});
    return 0;
}

static int run()
{
    M130_RUN(testBasicPushStats);
    M130_RUN(testNonMonotonicClamp);
    M130_RUN(testDownsample);
    return 0;
}

M130_TEST_MAIN()

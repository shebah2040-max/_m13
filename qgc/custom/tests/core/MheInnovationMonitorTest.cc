#include "analysis/MheInnovationMonitor.h"
#include "test_support.h"

#include <cmath>
#include <random>

using m130::analysis::MheInnovationMonitor;

static int testWhiteNoisePassesAfterWarmup()
{
    MheInnovationMonitor::Config cfg;
    cfg.warmup         = 50;
    cfg.mean_sigma     = 4.0;
    cfg.variance_tol   = 0.4;
    cfg.outlier_sigma  = 4.5;
    MheInnovationMonitor mon(cfg);

    std::mt19937 rng(42);
    std::normal_distribution<double> N01(0.0, 1.0);
    for (int i = 0; i < 500; ++i) {
        mon.push(N01(rng));
    }
    const auto& s = mon.status();
    M130_REQUIRE(s.samples == 500);
    M130_REQUIRE(!s.mean_bias);
    M130_REQUIRE(!s.variance_out);
    M130_REQUIRE(std::abs(s.running_mean) < 0.25);
    M130_REQUIRE(std::abs(s.running_var - 1.0) < 0.3);
    return 0;
}

static int testConstantBiasFlagsMean()
{
    MheInnovationMonitor::Config cfg;
    cfg.warmup = 10;
    cfg.mean_sigma = 3.0;
    MheInnovationMonitor mon(cfg);
    for (int i = 0; i < 100; ++i) {
        mon.push(0.5);  // clearly biased
    }
    const auto& s = mon.status();
    M130_REQUIRE(s.mean_bias);
    M130_REQUIRE(!s.reason.empty());
    return 0;
}

static int testVarianceDriftFlagged()
{
    MheInnovationMonitor::Config cfg;
    cfg.warmup = 10;
    cfg.variance_tol = 0.3;
    MheInnovationMonitor mon(cfg);
    std::mt19937 rng(7);
    std::normal_distribution<double> N(0.0, 3.0);
    for (int i = 0; i < 200; ++i) {
        mon.push(N(rng));
    }
    M130_REQUIRE(mon.status().variance_out);
    return 0;
}

static int testThreeSigmaExcursionFlag()
{
    MheInnovationMonitor::Config cfg;
    cfg.warmup = 5;
    cfg.outlier_sigma = 3.0;
    MheInnovationMonitor mon(cfg);
    for (int i = 0; i < 20; ++i) mon.push(0.1);
    mon.push(4.0);
    M130_REQUIRE(mon.status().three_sigma);
    mon.push(0.1);
    M130_REQUIRE(!mon.status().three_sigma);
    return 0;
}

static int testReset()
{
    MheInnovationMonitor mon;
    for (int i = 0; i < 10; ++i) mon.push(1.0);
    mon.reset();
    M130_REQUIRE(mon.status().samples == 0);
    M130_REQUIRE(mon.status().running_mean == 0.0);
    return 0;
}

static int run()
{
    M130_RUN(testWhiteNoisePassesAfterWarmup);
    M130_RUN(testConstantBiasFlagsMean);
    M130_RUN(testVarianceDriftFlagged);
    M130_RUN(testThreeSigmaExcursionFlag);
    M130_RUN(testReset);
    return 0;
}

M130_TEST_MAIN()

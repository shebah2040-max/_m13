#include "tuning/MpcTuningModel.h"
#include "test_support.h"

#include <cmath>
#include <string>

using m130::tuning::MpcTuningModel;
using m130::tuning::SetResult;
using m130::tuning::TuningParameterSpec;

static int testRegisterRejectsBadSpecs()
{
    MpcTuningModel m;
    M130_REQUIRE(!m.registerParameter({"", 0, 1, 0.5, 0, "", ""}));
    M130_REQUIRE(!m.registerParameter({"bad_range", 5, 1, 3, 0, "", ""}));
    M130_REQUIRE(!m.registerParameter({"bad_default", 0, 1, 2, 0, "", ""}));

    M130_REQUIRE(m.registerParameter({"k", 0.0, 10.0, 1.0, 0, "", ""}));
    M130_REQUIRE(!m.registerParameter({"k", 0.0, 10.0, 1.0, 0, "", ""}));
    return 0;
}

static int testStandardWeightsRegistered()
{
    MpcTuningModel m;
    m.registerStandardMpcWeights();
    M130_REQUIRE(m.contains("Q_attitude"));
    M130_REQUIRE(m.contains("R_fin"));
    M130_REQUIRE(m.contains("mhe_meas_r"));
    auto v = m.value("Q_attitude");
    M130_REQUIRE(v.has_value());
    M130_REQUIRE(std::abs(v.value() - 50.0) < 1e-12);
    return 0;
}

static int testSetOutOfRangeAndNan()
{
    MpcTuningModel m;
    m.registerStandardMpcWeights();

    double applied = 0.0;
    M130_REQUIRE_EQ(m.set("Q_attitude", -1.0, &applied), SetResult::OutOfRange);
    M130_REQUIRE_EQ(m.set("Q_attitude", 1e12, &applied), SetResult::OutOfRange);
    M130_REQUIRE_EQ(m.set("Q_attitude", std::nan(""), &applied), SetResult::OutOfRange);
    M130_REQUIRE_EQ(m.set("missing", 1.0, &applied), SetResult::UnknownParameter);
    M130_REQUIRE_EQ(m.value("Q_attitude").value(), 50.0);
    return 0;
}

static int testRateLimit()
{
    MpcTuningModel m;
    m.registerParameter({"x", 0.0, 100.0, 10.0, /*step*/ 5.0, "", ""});
    double applied = 0.0;
    M130_REQUIRE_EQ(m.set("x", 14.0, &applied), SetResult::Ok);
    M130_REQUIRE(std::abs(applied - 14.0) < 1e-12);
    M130_REQUIRE_EQ(m.set("x", 50.0, &applied), SetResult::RateLimited);
    M130_REQUIRE(std::abs(m.value("x").value() - 14.0) < 1e-12);
    return 0;
}

static int testSafetyGateRejects()
{
    MpcTuningModel m;
    m.registerStandardMpcWeights();
    bool called = false;
    m.setSafetyGate([&](std::string_view name, double, double) {
        called = true;
        return name != std::string_view("Q_attitude");
    });
    double applied = 0.0;
    M130_REQUIRE_EQ(m.set("Q_attitude", 60.0, &applied), SetResult::SafetyGated);
    M130_REQUIRE(called);
    M130_REQUIRE_EQ(m.set("R_fin", 0.2, &applied), SetResult::Ok);
    return 0;
}

static int testSnapshotRollback()
{
    MpcTuningModel m;
    m.registerStandardMpcWeights();
    double applied = 0.0;
    m.set("Q_attitude", 75.0, &applied);
    auto snap = m.snapshot("pre-change");
    m.set("Q_attitude", 90.0, &applied);
    M130_REQUIRE(std::abs(m.value("Q_attitude").value() - 90.0) < 1e-12);
    M130_REQUIRE(m.rollback(snap));
    M130_REQUIRE(std::abs(m.value("Q_attitude").value() - 75.0) < 1e-12);
    return 0;
}

static int testRollbackRejectsUnknownKeys()
{
    MpcTuningModel m;
    m.registerStandardMpcWeights();
    m130::tuning::TuningSnapshot bad;
    bad.values["does_not_exist"] = 1.0;
    M130_REQUIRE(!m.rollback(bad));
    return 0;
}

static int testResetToDefaults()
{
    MpcTuningModel m;
    m.registerStandardMpcWeights();
    double applied = 0.0;
    m.set("Q_attitude", 75.0, &applied);
    m.set("R_fin", 0.5, &applied);
    m.resetToDefaults();
    M130_REQUIRE(std::abs(m.value("Q_attitude").value() - 50.0) < 1e-12);
    M130_REQUIRE(std::abs(m.value("R_fin").value() - 0.1) < 1e-12);
    return 0;
}

static int run()
{
    M130_RUN(testRegisterRejectsBadSpecs);
    M130_RUN(testStandardWeightsRegistered);
    M130_RUN(testSetOutOfRangeAndNan);
    M130_RUN(testRateLimit);
    M130_RUN(testSafetyGateRejects);
    M130_RUN(testSnapshotRollback);
    M130_RUN(testRollbackRejectsUnknownKeys);
    M130_RUN(testResetToDefaults);
    return 0;
}

M130_TEST_MAIN()

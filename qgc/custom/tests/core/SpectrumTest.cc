#include "analysis/Spectrum.h"
#include "analysis/Window.h"
#include "test_support.h"

#include <cmath>
#include <vector>

using namespace m130::analysis;

static int testWindowSymmetryAndGain()
{
    for (WindowType t : {WindowType::Hann, WindowType::Hamming,
                         WindowType::Blackman, WindowType::BlackmanHarris}) {
        const auto w = makeWindow(t, 64);
        for (std::size_t n = 0; n < w.size() / 2; ++n) {
            M130_REQUIRE(std::abs(w[n] - w[w.size() - 1 - n]) < 1e-12);
        }
        const double cg = windowCoherentGain(w);
        const double en = windowEnbw(w);
        M130_REQUIRE(cg > 0.0 && cg < 1.0);
        M130_REQUIRE(en >= 1.0 && en < 3.0);
    }
    return 0;
}

static int testRectangularWindowIsAllOnes()
{
    const auto w = makeWindow(WindowType::Rectangular, 16);
    for (double v : w) {
        M130_REQUIRE(v == 1.0);
    }
    M130_REQUIRE(std::abs(windowCoherentGain(w) - 1.0) < 1e-12);
    M130_REQUIRE(std::abs(windowEnbw(w) - 1.0) < 1e-12);
    return 0;
}

static int testSpectrumFindsPeakAtInjectedFrequency()
{
    const double fs = 1000.0;    // 1 kHz sample rate
    const double f0 = 50.0;      // 50 Hz sine
    const std::size_t N = 1024;
    std::vector<double> x(N);
    for (std::size_t n = 0; n < N; ++n) {
        x[n] = std::sin(2.0 * 3.14159265358979323846 * f0 *
                        static_cast<double>(n) / fs);
    }
    auto s = computeSpectrum(x, fs, WindowType::Hann);
    M130_REQUIRE_EQ(s.frequencies_hz.size(), N / 2 + 1);
    M130_REQUIRE(s.df_hz > 0.0);

    const auto peak = findPeak(s);
    M130_REQUIRE(std::abs(peak.frequency_hz - f0) < s.df_hz * 1.5);
    // Magnitude should be close to the injected amplitude of 1.0 after
    // coherent-gain correction.
    M130_REQUIRE(peak.magnitude > 0.7 && peak.magnitude < 1.2);
    return 0;
}

static int testEmptyAndBadInputs()
{
    auto s = computeSpectrum({}, 1000.0, WindowType::Hann);
    M130_REQUIRE(s.frequencies_hz.empty());
    auto s2 = computeSpectrum({1.0, 2.0}, 0.0, WindowType::Hann);
    M130_REQUIRE(s2.frequencies_hz.empty());
    return 0;
}

static int run()
{
    M130_RUN(testWindowSymmetryAndGain);
    M130_RUN(testRectangularWindowIsAllOnes);
    M130_RUN(testSpectrumFindsPeakAtInjectedFrequency);
    M130_RUN(testEmptyAndBadInputs);
    return 0;
}

M130_TEST_MAIN()

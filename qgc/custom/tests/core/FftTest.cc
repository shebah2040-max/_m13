#include "analysis/Fft.h"
#include "test_support.h"

#include <cmath>
#include <vector>

using m130::analysis::Complex;
using m130::analysis::fft;
using m130::analysis::ifft;
using m130::analysis::fftReal;
using m130::analysis::isPowerOfTwo;
using m130::analysis::nextPowerOfTwo;

namespace {
constexpr double kPi = 3.14159265358979323846;

bool nearComplex(const Complex& a, const Complex& b, double tol = 1e-9)
{
    return std::abs(a.real() - b.real()) < tol &&
           std::abs(a.imag() - b.imag()) < tol;
}
} // namespace

static int testPowerOfTwoHelpers()
{
    M130_REQUIRE(isPowerOfTwo(1));
    M130_REQUIRE(isPowerOfTwo(2));
    M130_REQUIRE(isPowerOfTwo(1024));
    M130_REQUIRE(!isPowerOfTwo(0));
    M130_REQUIRE(!isPowerOfTwo(3));
    M130_REQUIRE(!isPowerOfTwo(1000));
    M130_REQUIRE_EQ(nextPowerOfTwo(5),   std::size_t{8});
    M130_REQUIRE_EQ(nextPowerOfTwo(8),   std::size_t{8});
    M130_REQUIRE_EQ(nextPowerOfTwo(513), std::size_t{1024});
    M130_REQUIRE_EQ(nextPowerOfTwo(1),   std::size_t{1});
    M130_REQUIRE_EQ(nextPowerOfTwo(0),   std::size_t{1});
    return 0;
}

static int testFftLength4KnownVector()
{
    // x = [1, 0, -1, 0] -> X = [0, 2, 0, 2]
    std::vector<Complex> x = {{1,0},{0,0},{-1,0},{0,0}};
    fft(x);
    M130_REQUIRE(nearComplex(x[0], {0.0, 0.0}));
    M130_REQUIRE(nearComplex(x[1], {2.0, 0.0}));
    M130_REQUIRE(nearComplex(x[2], {0.0, 0.0}));
    M130_REQUIRE(nearComplex(x[3], {2.0, 0.0}));
    return 0;
}

static int testFftMatchesDft()
{
    const std::size_t N = 16;
    std::vector<double> sig(N);
    for (std::size_t n = 0; n < N; ++n) {
        const double t = static_cast<double>(n) / static_cast<double>(N);
        sig[n] = std::sin(2.0 * kPi * 3.0 * t) +
                 0.5 * std::cos(2.0 * kPi * 5.0 * t);
    }
    auto X = fftReal(sig);

    // Reference DFT.
    std::vector<Complex> ref(N, {0.0, 0.0});
    for (std::size_t k = 0; k < N; ++k) {
        Complex acc(0.0, 0.0);
        for (std::size_t n = 0; n < N; ++n) {
            const double ang = -2.0 * kPi * static_cast<double>(k * n) /
                               static_cast<double>(N);
            acc += Complex(std::cos(ang), std::sin(ang)) * sig[n];
        }
        ref[k] = acc;
    }
    for (std::size_t k = 0; k < N; ++k) {
        M130_REQUIRE(nearComplex(X[k], ref[k], 1e-6));
    }
    return 0;
}

static int testFftIfftRoundtrip()
{
    std::vector<Complex> x = {
        {1.0, 0.0}, {0.7, 0.3}, {-0.2, 0.5}, {0.9, -0.1},
        {0.0, 0.0}, {-1.0, 0.0}, {0.4, 0.4}, {0.2, -0.2}
    };
    auto copy = x;
    fft(copy);
    ifft(copy);
    for (std::size_t i = 0; i < x.size(); ++i) {
        M130_REQUIRE(nearComplex(x[i], copy[i], 1e-9));
    }
    return 0;
}

static int testFftRejectsNonPow2()
{
    std::vector<Complex> x(6, {0.0, 0.0});
    bool threw = false;
    try { fft(x); } catch (const std::invalid_argument&) { threw = true; }
    M130_REQUIRE(threw);
    return 0;
}

static int run()
{
    M130_RUN(testPowerOfTwoHelpers);
    M130_RUN(testFftLength4KnownVector);
    M130_RUN(testFftMatchesDft);
    M130_RUN(testFftIfftRoundtrip);
    M130_RUN(testFftRejectsNonPow2);
    return 0;
}

M130_TEST_MAIN()

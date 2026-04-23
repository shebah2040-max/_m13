#include "Window.h"

#include <cmath>

namespace m130::analysis {

namespace {
constexpr double kPi = 3.14159265358979323846;
}

std::vector<double> makeWindow(WindowType type, std::size_t N)
{
    std::vector<double> w(N, 1.0);
    if (N <= 1) {
        return w;
    }
    const double denom = static_cast<double>(N - 1);
    for (std::size_t n = 0; n < N; ++n) {
        const double x = static_cast<double>(n) / denom;
        switch (type) {
        case WindowType::Rectangular:
            w[n] = 1.0;
            break;
        case WindowType::Hann:
            w[n] = 0.5 * (1.0 - std::cos(2.0 * kPi * x));
            break;
        case WindowType::Hamming:
            w[n] = 0.54 - 0.46 * std::cos(2.0 * kPi * x);
            break;
        case WindowType::Blackman:
            w[n] = 0.42 - 0.5  * std::cos(2.0 * kPi * x)
                        + 0.08 * std::cos(4.0 * kPi * x);
            break;
        case WindowType::BlackmanHarris:
            w[n] = 0.35875 - 0.48829 * std::cos(2.0 * kPi * x)
                           + 0.14128 * std::cos(4.0 * kPi * x)
                           - 0.01168 * std::cos(6.0 * kPi * x);
            break;
        }
    }
    return w;
}

double windowCoherentGain(const std::vector<double>& w)
{
    if (w.empty()) {
        return 0.0;
    }
    double s = 0.0;
    for (double v : w) {
        s += v;
    }
    return s / static_cast<double>(w.size());
}

double windowEnbw(const std::vector<double>& w)
{
    if (w.empty()) {
        return 0.0;
    }
    double s1 = 0.0;
    double s2 = 0.0;
    for (double v : w) {
        s1 += v;
        s2 += v * v;
    }
    if (s1 == 0.0) {
        return 0.0;
    }
    return s2 * static_cast<double>(w.size()) / (s1 * s1);
}

} // namespace m130::analysis

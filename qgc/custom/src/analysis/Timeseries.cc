#include "Timeseries.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace m130::analysis {

Timeseries::Timeseries(std::string name, std::size_t capacity)
    : _name(std::move(name)), _ring(capacity)
{
}

void Timeseries::push(std::uint64_t t_ms, double value)
{
    // Clamp non-monotonic timestamps up to the last seen t to preserve
    // chronological ordering invariants for downstream FFT/interpolation.
    if (t_ms < _last_t_ms) {
        t_ms = _last_t_ms;
    }
    _last_t_ms = t_ms;
    _ring.push(Sample{t_ms, value});
}

std::vector<Sample> Timeseries::downsample(std::size_t max_points) const
{
    const std::size_t n = _ring.size();
    if (n == 0 || max_points == 0) {
        return {};
    }
    if (n <= max_points) {
        return _ring.snapshot();
    }

    std::vector<Sample> out;
    out.reserve(max_points);
    for (std::size_t i = 0; i < max_points; ++i) {
        const std::size_t idx = (i * (n - 1)) / (max_points - 1);
        out.push_back(_ring.at(idx));
    }
    return out;
}

double Timeseries::mean() const
{
    const std::size_t n = _ring.size();
    if (n == 0) {
        return 0.0;
    }
    double acc = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        acc += _ring.at(i).value;
    }
    return acc / static_cast<double>(n);
}

double Timeseries::stddev() const
{
    const std::size_t n = _ring.size();
    if (n < 2) {
        return 0.0;
    }
    const double m = mean();
    double acc = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double d = _ring.at(i).value - m;
        acc += d * d;
    }
    return std::sqrt(acc / static_cast<double>(n - 1));
}

double Timeseries::minVal() const
{
    const std::size_t n = _ring.size();
    if (n == 0) {
        return 0.0;
    }
    double v = std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < n; ++i) {
        v = std::min(v, _ring.at(i).value);
    }
    return v;
}

double Timeseries::maxVal() const
{
    const std::size_t n = _ring.size();
    if (n == 0) {
        return 0.0;
    }
    double v = -std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < n; ++i) {
        v = std::max(v, _ring.at(i).value);
    }
    return v;
}

} // namespace m130::analysis

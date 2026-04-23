#pragma once

#include <cstddef>
#include <vector>

namespace m130::analysis {

// Window functions indexed by n in [0, N-1]. All functions are symmetric
// about (N-1)/2 and non-negative. Coefficients follow the conventional
// definitions in Harris (1978) and Blackman/Tukey.

enum class WindowType {
    Rectangular,
    Hann,
    Hamming,
    Blackman,
    BlackmanHarris
};

std::vector<double> makeWindow(WindowType type, std::size_t N);

// Coherent gain sum(w[n])/N — used to un-bias windowed FFT magnitude.
double windowCoherentGain(const std::vector<double>& w);

// Equivalent noise bandwidth sum(w^2)/(sum(w))^2 * N — used to normalise
// PSD estimates.
double windowEnbw(const std::vector<double>& w);

} // namespace m130::analysis

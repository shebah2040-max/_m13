#pragma once

#include "Window.h"

#include <cstddef>
#include <vector>

namespace m130::analysis {

struct SpectrumResult {
    std::vector<double> frequencies_hz;  // size N/2 + 1
    std::vector<double> magnitude;       // amplitude of bin (unitless)
    std::vector<double> psd;             // power spectral density V^2/Hz
    double              df_hz = 0.0;     // bin width
    double              enbw  = 0.0;     // window equivalent noise bandwidth
};

// One-sided spectrum of a real signal sampled at sample_rate_hz. Zero-pads
// the input to nextPowerOfTwo. Applies the chosen window and corrects
// magnitude/PSD per standard formulas (Harris 1978).
SpectrumResult computeSpectrum(const std::vector<double>& x,
                               double sample_rate_hz,
                               WindowType window = WindowType::Hann);

// Peak bin in the magnitude spectrum (DC bin excluded).
struct SpectrumPeak {
    double frequency_hz = 0.0;
    double magnitude    = 0.0;
};
SpectrumPeak findPeak(const SpectrumResult& s);

} // namespace m130::analysis

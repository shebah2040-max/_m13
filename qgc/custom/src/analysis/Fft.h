#pragma once

#include <complex>
#include <cstddef>
#include <vector>

namespace m130::analysis {

// Cooley-Tukey radix-2 in-place FFT. Requires N to be a power of two.
// Throws std::invalid_argument if N is not a power of two.

using Complex = std::complex<double>;

bool isPowerOfTwo(std::size_t n) noexcept;
std::size_t nextPowerOfTwo(std::size_t n) noexcept;

void fft(std::vector<Complex>& x);
void ifft(std::vector<Complex>& x);

// Convenience: forward FFT on a real signal. Output has size
// N = nextPowerOfTwo(input.size()); input is zero-padded if needed.
std::vector<Complex> fftReal(const std::vector<double>& x);

} // namespace m130::analysis

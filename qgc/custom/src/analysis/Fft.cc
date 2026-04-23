#include "Fft.h"

#include <cmath>
#include <stdexcept>

namespace m130::analysis {

namespace {
constexpr double kPi = 3.14159265358979323846;

void bitReverseReorder(std::vector<Complex>& x)
{
    const std::size_t n = x.size();
    std::size_t j = 0;
    for (std::size_t i = 1; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(x[i], x[j]);
        }
    }
}

void transformInternal(std::vector<Complex>& x, bool inverse)
{
    const std::size_t n = x.size();
    if (n == 0) {
        return;
    }
    if (!isPowerOfTwo(n)) {
        throw std::invalid_argument("FFT length must be a power of two");
    }
    bitReverseReorder(x);
    for (std::size_t len = 2; len <= n; len <<= 1) {
        const double ang = (inverse ? 2.0 : -2.0) * kPi / static_cast<double>(len);
        const Complex wlen(std::cos(ang), std::sin(ang));
        const std::size_t half = len >> 1;
        for (std::size_t i = 0; i < n; i += len) {
            Complex w(1.0, 0.0);
            for (std::size_t k = 0; k < half; ++k) {
                const Complex u = x[i + k];
                const Complex t = w * x[i + k + half];
                x[i + k]        = u + t;
                x[i + k + half] = u - t;
                w *= wlen;
            }
        }
    }
    if (inverse) {
        const double inv_n = 1.0 / static_cast<double>(n);
        for (auto& v : x) {
            v *= inv_n;
        }
    }
}
} // namespace

bool isPowerOfTwo(std::size_t n) noexcept
{
    return n != 0 && (n & (n - 1)) == 0;
}

std::size_t nextPowerOfTwo(std::size_t n) noexcept
{
    if (n <= 1) {
        return 1;
    }
    std::size_t p = 1;
    while (p < n) {
        p <<= 1;
    }
    return p;
}

void fft(std::vector<Complex>& x)  { transformInternal(x, false); }
void ifft(std::vector<Complex>& x) { transformInternal(x, true); }

std::vector<Complex> fftReal(const std::vector<double>& x)
{
    const std::size_t n = nextPowerOfTwo(x.size());
    std::vector<Complex> out(n, Complex(0.0, 0.0));
    for (std::size_t i = 0; i < x.size(); ++i) {
        out[i] = Complex(x[i], 0.0);
    }
    fft(out);
    return out;
}

} // namespace m130::analysis

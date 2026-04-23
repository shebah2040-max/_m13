#include "Spectrum.h"

#include "Fft.h"
#include "Window.h"

#include <cmath>

namespace m130::analysis {

SpectrumResult computeSpectrum(const std::vector<double>& x,
                               double sample_rate_hz,
                               WindowType window)
{
    SpectrumResult out;
    if (x.empty() || sample_rate_hz <= 0.0) {
        return out;
    }
    const std::size_t N    = nextPowerOfTwo(x.size());
    const auto        w    = makeWindow(window, x.size());
    const double      cg   = windowCoherentGain(w);
    const double      enbw = windowEnbw(w);

    std::vector<Complex> buf(N, Complex(0.0, 0.0));
    for (std::size_t i = 0; i < x.size(); ++i) {
        buf[i] = Complex(x[i] * w[i], 0.0);
    }
    fft(buf);

    const std::size_t half = N / 2 + 1;
    const double      df   = sample_rate_hz / static_cast<double>(N);

    out.frequencies_hz.resize(half);
    out.magnitude.resize(half);
    out.psd.resize(half);
    out.df_hz = df;
    out.enbw  = enbw;

    const double gain_corr  = (cg > 0.0) ? 1.0 / cg : 1.0;
    const double psd_denom  = sample_rate_hz * static_cast<double>(x.size()) *
                              (cg * cg) * enbw;

    for (std::size_t k = 0; k < half; ++k) {
        const double re  = buf[k].real();
        const double im  = buf[k].imag();
        const double m2  = re * re + im * im;
        const double amp = std::sqrt(m2) / static_cast<double>(N);
        // Double non-DC / non-Nyquist bins for one-sided spectrum.
        const double one_sided =
            (k == 0 || (k == N / 2 && N % 2 == 0)) ? 1.0 : 2.0;
        out.frequencies_hz[k] = static_cast<double>(k) * df;
        out.magnitude[k]      = amp * gain_corr * one_sided;
        if (psd_denom > 0.0) {
            out.psd[k] = m2 * one_sided / psd_denom;
        } else {
            out.psd[k] = 0.0;
        }
    }
    return out;
}

SpectrumPeak findPeak(const SpectrumResult& s)
{
    SpectrumPeak p;
    for (std::size_t k = 1; k < s.magnitude.size(); ++k) {
        if (s.magnitude[k] > p.magnitude) {
            p.magnitude    = s.magnitude[k];
            p.frequency_hz = s.frequencies_hz[k];
        }
    }
    return p;
}

} // namespace m130::analysis

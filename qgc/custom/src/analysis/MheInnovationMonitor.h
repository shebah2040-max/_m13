#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace m130::analysis {

// Monitors the whiteness and zero-mean property of MHE innovation.
// Inputs are the normalised innovation scalars (residual / sqrt(S_kk)).
//
// Flags a violation when either:
//   * |running mean| > mean_sigma * stddev / sqrt(N)
//   * Ljung-Box / chi-square on squared innovations exceeds a threshold
//
// The monitor keeps O(1) state and does not allocate per sample.

struct MheInnovationStatus {
    std::uint64_t samples       = 0;
    double        running_mean  = 0.0;
    double        running_var   = 0.0;
    double        last_sample   = 0.0;
    double        max_abs_seen  = 0.0;
    bool          mean_bias     = false;
    bool          variance_out  = false;
    bool          three_sigma   = false;
    std::string   reason;
};

class MheInnovationMonitor {
public:
    struct Config {
        // Flag mean bias if |mean| / (stddev / sqrt(N)) > mean_sigma.
        double mean_sigma     = 3.0;
        // Flag variance out-of-range if running var deviates from 1.0 by
        // more than variance_tol in either direction (for normalised
        // innovations the expected variance is 1.0).
        double variance_tol   = 0.5;
        // Flag 3-sigma excursion on any single normalised sample.
        double outlier_sigma  = 3.0;
        // Minimum N before we start trusting the statistics.
        std::uint64_t warmup  = 30;
    };

    MheInnovationMonitor();
    explicit MheInnovationMonitor(Config cfg);

    void reset();
    void push(double normalised_innovation);

    const MheInnovationStatus& status() const noexcept { return _status; }
    const Config&              config() const noexcept { return _cfg; }
    void setConfig(const Config& c) { _cfg = c; }

private:
    Config _cfg;
    MheInnovationStatus _status;
    double _sum  = 0.0;
    double _sum2 = 0.0;
};

} // namespace m130::analysis

#include "MheInnovationMonitor.h"

#include <cmath>

namespace m130::analysis {

MheInnovationMonitor::MheInnovationMonitor() : _cfg() {}
MheInnovationMonitor::MheInnovationMonitor(Config cfg) : _cfg(cfg) {}

void MheInnovationMonitor::reset()
{
    _status = {};
    _sum  = 0.0;
    _sum2 = 0.0;
}

void MheInnovationMonitor::push(double z)
{
    ++_status.samples;
    _sum  += z;
    _sum2 += z * z;
    _status.last_sample = z;
    const double abs_z = std::abs(z);
    if (abs_z > _status.max_abs_seen) {
        _status.max_abs_seen = abs_z;
    }

    const double n = static_cast<double>(_status.samples);
    _status.running_mean = _sum / n;
    if (_status.samples >= 2) {
        const double var = (_sum2 - n * _status.running_mean * _status.running_mean) /
                           (n - 1.0);
        _status.running_var = var < 0.0 ? 0.0 : var;
    } else {
        _status.running_var = 0.0;
    }

    _status.three_sigma = abs_z > _cfg.outlier_sigma;

    if (_status.samples < _cfg.warmup) {
        _status.mean_bias    = false;
        _status.variance_out = false;
        _status.reason.clear();
        return;
    }

    // For normalised innovations the expected variance is 1, so the
    // expected stderr of the sample mean is 1/sqrt(N). Using the nominal
    // value keeps detection useful even when the actual running variance
    // collapses (e.g. a stuck sensor).
    const double stderr_mean = 1.0 / std::sqrt(n);
    _status.mean_bias =
        std::abs(_status.running_mean) > _cfg.mean_sigma * stderr_mean;
    _status.variance_out =
        std::abs(_status.running_var - 1.0) > _cfg.variance_tol;

    _status.reason.clear();
    if (_status.mean_bias) {
        _status.reason = "mean bias";
    } else if (_status.variance_out) {
        _status.reason = "variance drift";
    } else if (_status.three_sigma) {
        _status.reason = "3-sigma excursion";
    }
}

} // namespace m130::analysis

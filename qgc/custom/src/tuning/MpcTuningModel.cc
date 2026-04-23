#include "MpcTuningModel.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace m130::tuning {

MpcTuningModel::MpcTuningModel() = default;

bool MpcTuningModel::registerParameter(TuningParameterSpec spec)
{
    if (spec.name.empty() || spec.min_value > spec.max_value) {
        return false;
    }
    if (spec.default_value < spec.min_value || spec.default_value > spec.max_value) {
        return false;
    }
    const std::string key = spec.name;
    if (_entries.find(key) != _entries.end()) {
        return false;
    }
    _entries.emplace(key, Entry{std::move(spec), 0.0});
    auto& e = _entries[key];
    e.value = e.spec.default_value;
    return true;
}

void MpcTuningModel::registerStandardMpcWeights()
{
    // Baseline weights mirror the Python reference in
    // m13/6DOF_v4_pure/mpc/m130_mpc_autopilot.py. Bounds are intentionally
    // conservative; raising them requires a formal review + SCMP change.
    registerParameter({"Q_attitude",     1.0,    1000.0, 50.0,   50.0,  "rad^-2", "Attitude tracking weight"});
    registerParameter({"Q_rate",         0.01,   100.0,   5.0,    5.0,  "(rad/s)^-2", "Body-rate tracking weight"});
    registerParameter({"Q_accel",        0.1,    500.0,  20.0,   20.0,  "(m/s^2)^-2", "Longitudinal accel weight"});
    registerParameter({"Q_crossrange",   0.1,    500.0,  10.0,   10.0,  "m^-2", "Crossrange tracking weight"});
    registerParameter({"R_fin",          0.001,   10.0,   0.1,    0.1,  "rad^-2", "Fin effort penalty"});
    registerParameter({"R_fin_rate",     0.001,   10.0,   0.5,    0.5,  "(rad/s)^-2", "Fin rate penalty"});
    registerParameter({"Q_terminal",     1.0,   10000.0, 200.0, 200.0,  "-", "Terminal cost multiplier"});
    registerParameter({"mhe_process_q",  1e-4,    10.0,   0.01,   0.01, "-", "MHE process noise variance"});
    registerParameter({"mhe_meas_r",     1e-4,    10.0,   0.05,   0.05, "-", "MHE measurement noise variance"});
}

std::vector<std::string> MpcTuningModel::names() const
{
    std::vector<std::string> out;
    out.reserve(_entries.size());
    for (const auto& kv : _entries) {
        out.push_back(kv.first);
    }
    std::sort(out.begin(), out.end());
    return out;
}

std::optional<TuningParameterSpec> MpcTuningModel::spec(std::string_view name) const
{
    auto it = _entries.find(std::string(name));
    if (it == _entries.end()) {
        return std::nullopt;
    }
    return it->second.spec;
}

std::optional<double> MpcTuningModel::value(std::string_view name) const
{
    auto it = _entries.find(std::string(name));
    if (it == _entries.end()) {
        return std::nullopt;
    }
    return it->second.value;
}

bool MpcTuningModel::contains(std::string_view name) const
{
    return _entries.find(std::string(name)) != _entries.end();
}

SetResult MpcTuningModel::set(std::string_view name, double requested, double* applied)
{
    auto it = _entries.find(std::string(name));
    if (it == _entries.end()) {
        return SetResult::UnknownParameter;
    }
    Entry& e = it->second;
    if (!std::isfinite(requested)) {
        return SetResult::OutOfRange;
    }
    if (requested < e.spec.min_value || requested > e.spec.max_value) {
        return SetResult::OutOfRange;
    }
    if (e.spec.max_step > 0.0 &&
        std::abs(requested - e.value) > e.spec.max_step) {
        return SetResult::RateLimited;
    }
    if (_gate && !_gate(e.spec.name, e.value, requested)) {
        return SetResult::SafetyGated;
    }
    e.value = requested;
    if (applied) {
        *applied = requested;
    }
    return SetResult::Ok;
}

void MpcTuningModel::resetToDefaults()
{
    for (auto& kv : _entries) {
        kv.second.value = kv.second.spec.default_value;
    }
}

TuningSnapshot MpcTuningModel::snapshot(std::string label) const
{
    TuningSnapshot s;
    s.t_ms  = nowMs();
    s.label = std::move(label);
    s.values.reserve(_entries.size());
    for (const auto& kv : _entries) {
        s.values.emplace(kv.first, kv.second.value);
    }
    return s;
}

bool MpcTuningModel::rollback(const TuningSnapshot& s)
{
    for (const auto& kv : s.values) {
        if (_entries.find(kv.first) == _entries.end()) {
            return false;
        }
    }
    for (const auto& kv : s.values) {
        auto it = _entries.find(kv.first);
        const auto& spec = it->second.spec;
        const double v = std::clamp(kv.second, spec.min_value, spec.max_value);
        it->second.value = v;
    }
    return true;
}

} // namespace m130::tuning

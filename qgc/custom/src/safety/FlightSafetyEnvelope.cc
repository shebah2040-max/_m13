#include "FlightSafetyEnvelope.h"

namespace m130::safety {

AlertLevel FlightSafetyEnvelope::classify(double v, const EnvelopeBounds& b)
{
    if (v < b.low_emergency  || v > b.high_emergency)  return AlertLevel::Emergency;
    if (v < b.low_warning    || v > b.high_warning)    return AlertLevel::Warning;
    if (v < b.low_caution    || v > b.high_caution)    return AlertLevel::Caution;
    if (v < b.low_advisory   || v > b.high_advisory)   return AlertLevel::Advisory;
    return AlertLevel::None;
}

void FlightSafetyEnvelope::setEnvelope(VariableEnvelope env)
{
    std::string key = env.variable;
    _envs[std::move(key)] = std::move(env);
}

const EnvelopeBounds& FlightSafetyEnvelope::boundsFor(const VariableEnvelope& ve, FlightPhase phase) const
{
    auto it = ve.per_phase.find(phase);
    if (it != ve.per_phase.end()) {
        return it->second;
    }
    return ve.fallback;
}

EnvelopeCheckResult FlightSafetyEnvelope::check(std::string_view variable, double value, FlightPhase phase) const
{
    EnvelopeCheckResult r;
    r.variable = std::string(variable);
    r.value    = value;
    r.phase    = phase;

    auto it = _envs.find(r.variable);
    if (it == _envs.end()) {
        r.level = AlertLevel::None; // not monitored
        emit(r);
        return r;
    }
    r.level = classify(value, boundsFor(it->second, phase));
    emit(r);
    return r;
}

std::vector<EnvelopeCheckResult> FlightSafetyEnvelope::checkMany(
    const std::unordered_map<std::string, double>& samples, FlightPhase phase) const
{
    std::vector<EnvelopeCheckResult> out;
    out.reserve(samples.size());
    for (const auto& [name, v] : samples) {
        out.push_back(check(name, v, phase));
    }
    return out;
}

void FlightSafetyEnvelope::emit(const EnvelopeCheckResult& r) const
{
    for (const auto& s : _sinks) {
        if (s) s(r);
    }
}

FlightSafetyEnvelope FlightSafetyEnvelope::createDefault()
{
    FlightSafetyEnvelope e;

    {
        VariableEnvelope v;
        v.variable = "phi";
        EnvelopeBounds b;
        b.low_emergency = -180; b.low_warning = -90; b.low_caution = -45; b.low_advisory = -20;
        b.high_advisory = 20; b.high_caution = 45; b.high_warning = 90; b.high_emergency = 180;
        v.fallback = b;
        e.setEnvelope(std::move(v));
    }
    {
        VariableEnvelope v;
        v.variable = "airspeed";
        EnvelopeBounds b;
        b.low_emergency = -1; b.low_warning = -1; b.low_caution = -1; b.low_advisory = -1;
        b.high_advisory = 800; b.high_caution = 900; b.high_warning = 1000; b.high_emergency = 1100;
        v.fallback = b;
        e.setEnvelope(std::move(v));
    }
    {
        VariableEnvelope v;
        v.variable = "alpha_est";
        EnvelopeBounds b;
        b.low_emergency = -15; b.low_warning = -12; b.low_caution = -8; b.low_advisory = -5;
        b.high_advisory = 5; b.high_caution = 8; b.high_warning = 12; b.high_emergency = 15;
        v.fallback = b;
        e.setEnvelope(std::move(v));
    }
    {
        VariableEnvelope v;
        v.variable = "mpc_solve_us";
        EnvelopeBounds b;
        b.low_emergency = -1; b.low_warning = -1; b.low_caution = -1; b.low_advisory = -1;
        b.high_advisory = 3000; b.high_caution = 5000; b.high_warning = 8000; b.high_emergency = 15000;
        v.fallback = b;
        e.setEnvelope(std::move(v));
    }
    {
        VariableEnvelope v;
        v.variable = "mhe_quality";
        EnvelopeBounds b;
        b.low_emergency = 0.2; b.low_warning = 0.4; b.low_caution = 0.6; b.low_advisory = 0.8;
        b.high_advisory = 1e6; b.high_caution = 1e6; b.high_warning = 1e6; b.high_emergency = 1e6;
        v.fallback = b;
        e.setEnvelope(std::move(v));
    }

    return e;
}

} // namespace m130::safety

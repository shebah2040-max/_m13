#pragma once

#include "AlertLevel.h"
#include "FlightPhase.h"

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace m130::safety {

/// A single variable's nested thresholds. Semantics:
///   value < low_emergency                   → Emergency
///   value < low_warning                     → Warning
///   value < low_caution                     → Caution
///   value < low_advisory                    → Advisory
///   low_advisory <= value <= high_advisory  → None (nominal)
///   value > high_advisory                   → Advisory
///   ... (symmetric)
struct EnvelopeBounds {
    double low_emergency  = -1e300;
    double low_warning    = -1e300;
    double low_caution    = -1e300;
    double low_advisory   = -1e300;
    double high_advisory  =  1e300;
    double high_caution   =  1e300;
    double high_warning   =  1e300;
    double high_emergency =  1e300;
};

/// Phase-dependent envelope: per-variable bounds for each phase.
struct VariableEnvelope {
    std::string variable;
    std::unordered_map<FlightPhase, EnvelopeBounds> per_phase;
    EnvelopeBounds fallback;   ///< used when phase not listed
};

struct EnvelopeCheckResult {
    std::string variable;
    double      value = 0.0;
    AlertLevel  level = AlertLevel::None;
    FlightPhase phase = FlightPhase::Unknown;
};

/// Central envelope service (REQ-M130-GCS-SAFE-007 support, REQ-M130-GCS-TELE-003).
class FlightSafetyEnvelope
{
public:
    using Sink = std::function<void(const EnvelopeCheckResult&)>;

    FlightSafetyEnvelope() = default;

    /// Load or replace the envelope spec for a variable.
    void setEnvelope(VariableEnvelope env);

    /// Evaluate a single sample for a given phase.
    EnvelopeCheckResult check(std::string_view variable, double value, FlightPhase phase) const;

    /// Apply many samples at once.
    std::vector<EnvelopeCheckResult> checkMany(
        const std::unordered_map<std::string, double>& samples,
        FlightPhase phase) const;

    /// Subscribe to check results (useful for routing to AlertManager).
    void subscribe(Sink s) { _sinks.push_back(std::move(s)); }

    const std::unordered_map<std::string, VariableEnvelope>& envelopes() const noexcept { return _envs; }

    /// Foundation default envelope (subset of docs/safety/SafetyEnvelope.md).
    static FlightSafetyEnvelope createDefault();

    /// Classify a sample using the given bounds.
    static AlertLevel classify(double value, const EnvelopeBounds& b);

private:
    std::unordered_map<std::string, VariableEnvelope> _envs;
    std::vector<Sink> _sinks;

    const EnvelopeBounds& boundsFor(const VariableEnvelope& ve, FlightPhase phase) const;
    void emit(const EnvelopeCheckResult& r) const;
};

} // namespace m130::safety

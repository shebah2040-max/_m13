#pragma once

#include "TuningParameter.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace m130::tuning {

// Aggregates all operator-tunable MPC weights with hard bounds and an
// optional safety-gate predicate. The gate is consulted on every `set`;
// when it returns false the change is rejected and must be re-attempted
// after a step-up re-auth (see Pillar 6 SessionManager::stepUp).
//
// Snapshots are immutable maps of parameter value at a point in time and
// can be written to the FDR (Pillar 4) as a "tuning commit" event for
// post-flight replay and audit.

struct TuningSnapshot {
    std::uint64_t                           t_ms = 0;
    std::string                             label;
    std::unordered_map<std::string, double> values;
};

class MpcTuningModel {
public:
    using SafetyGate = std::function<bool(std::string_view name,
                                          double current_value,
                                          double requested_value)>;
    using Clock      = std::function<std::uint64_t()>;

    MpcTuningModel();

    // Registers the parameter and seeds it with its default value. The
    // name must be unique. Returns false when a parameter with that name
    // already exists.
    bool registerParameter(TuningParameterSpec spec);

    // Canonical spec registration used for MPC autopilot weights.
    void registerStandardMpcWeights();

    std::vector<std::string>       names() const;
    std::optional<TuningParameterSpec> spec(std::string_view name) const;
    std::optional<double>          value(std::string_view name) const;
    bool                           contains(std::string_view name) const;

    // Attempts to set the named parameter. Returns Ok on success. When
    // return is Ok, `*applied` (if non-null) is filled with the new
    // value. When non-Ok, `*applied` is unchanged.
    SetResult set(std::string_view name,
                  double requested_value,
                  double* applied = nullptr);

    // Reset to default values for every registered parameter.
    void resetToDefaults();

    // Snapshot + rollback. `rollback` returns false if the snapshot
    // references unknown parameter names.
    TuningSnapshot snapshot(std::string label = {}) const;
    bool           rollback(const TuningSnapshot& s);

    // Safety integration — if set, `set` is rejected with SafetyGated
    // when the gate returns false. The gate can inspect the mission
    // state, active alerts, session step-up status, etc.
    void setSafetyGate(SafetyGate g) { _gate = std::move(g); }
    void setClock(Clock c)           { _clock = std::move(c); }

    std::uint64_t nowMs() const { return _clock ? _clock() : 0; }

private:
    struct Entry {
        TuningParameterSpec spec;
        double              value;
    };

    std::unordered_map<std::string, Entry> _entries;
    SafetyGate  _gate;
    Clock       _clock;
};

} // namespace m130::tuning

#pragma once

#include "FlightPhase.h"

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace m130::safety {

/// Reason a transition request may be rejected.
enum class TransitionResult : uint8_t {
    Accepted       = 0,
    RejectedIllegal     = 1, ///< (from,to) not in the transition table
    RejectedGuard       = 2, ///< Transition legal but guard condition unmet
    RejectedTerminal    = 3, ///< Source phase is terminal (Landed)
    RejectedNoChange    = 4, ///< to == current
};

struct TransitionRecord {
    uint64_t timestamp_ms = 0;
    FlightPhase from       = FlightPhase::Unknown;
    FlightPhase to         = FlightPhase::Unknown;
    TransitionResult result = TransitionResult::Accepted;
    std::string reason;
};

/// Pure C++ mission state machine. Testable without Qt.
///
/// Covers REQ-M130-GCS-SAFE-001 (formal mission state machine) and
/// REQ-M130-GCS-SAFE-002 (guarded transitions with rejection logging).
///
/// Transitions are defined by a whitelist table. A transition is accepted
/// only when BOTH:
///   1. (current, target) is in the legal transition table
///   2. The registered guard (if any) returns true
/// ABORT is reachable from ANY in-flight phase regardless of guards (safety
/// override). Landed is terminal and accepts no further transitions.
class MissionStateMachine
{
public:
    using Guard = std::function<bool(FlightPhase from, FlightPhase to)>;
    using Listener = std::function<void(const TransitionRecord&)>;
    using Clock = std::function<uint64_t()>; ///< monotonic ms

    MissionStateMachine();

    /// Construct with injected clock for deterministic tests.
    explicit MissionStateMachine(Clock clock);

    FlightPhase current() const noexcept { return _current; }

    /// Request a transition. Returns the outcome and, on success, updates
    /// @c current() and appends a record to @c history().
    TransitionResult requestTransition(FlightPhase target, std::string_view reason = {});

    /// Set a guard for a specific transition. A single guard per (from,to).
    void setGuard(FlightPhase from, FlightPhase to, Guard g);

    /// Register a listener invoked for EVERY request (accepted or rejected).
    void addListener(Listener l);

    /// Full ordered history of requests since construction.
    const std::vector<TransitionRecord>& history() const noexcept { return _history; }

    /// Check if @p (from,to) is in the legal transition table.
    static bool isLegal(FlightPhase from, FlightPhase to);

private:
    Clock    _clock;
    FlightPhase _current = FlightPhase::Unknown;
    std::vector<TransitionRecord> _history;
    std::vector<Listener> _listeners;

    struct GuardEntry {
        FlightPhase from;
        FlightPhase to;
        Guard g;
    };
    std::vector<GuardEntry> _guards;

    void notify(const TransitionRecord& r);
};

} // namespace m130::safety

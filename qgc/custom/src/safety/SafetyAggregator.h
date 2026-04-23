#pragma once

#include "AlertManager.h"
#include "CommandAuthorization.h"
#include "FlightPhase.h"
#include "FlightSafetyEnvelope.h"
#include "FlightTerminationService.h"
#include "MissionStateMachine.h"
#include "Watchdog.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace m130::safety {

/// Qt-free aggregator that owns and wires together the six Safety Kernel
/// components. This is the class tested in `tests/core/`; the Qt-bound
/// `SafetyKernel` is a thin QObject wrapper around it.
///
/// Routing summary (covers REQ-M130-GCS-SAFE-004 / SAFE-005 / SAFE-007):
///
///   Watchdog.Event  ─┐
///   Envelope.Result ─┼─► AlertManager  ─► Sink subscribers (QML model, audit log)
///   MissionSM.Txn   ─┘
///
/// Lifecycle:
///   SafetyAggregator agg(clock);
///   agg.installDefaults();           // default envelope + command policy
///   agg.watchdog().addChannel(...);
///   agg.subscribeAlerts([](const Alert& a, bool ack){ ... });
class SafetyAggregator
{
public:
    using Clock = std::function<uint64_t()>;

    SafetyAggregator();
    explicit SafetyAggregator(Clock clock);

    SafetyAggregator(const SafetyAggregator&) = delete;
    SafetyAggregator& operator=(const SafetyAggregator&) = delete;

    /// Install the default envelope and command-authorisation policy.
    /// Safe to call once after construction.
    void installDefaults();

    /// Register a heartbeat-style channel with the watchdog.
    /// Helper for typical telemetry sources (heartbeat, gnc_state, etc.).
    void addChannel(std::string name,
                    StalenessThresholds t = defaultHeartbeatThresholds());

    // ─────── Inputs (call these when data arrives) ────────────────────────

    /// Report a fresh sample for a watchdog channel.
    void feed(std::string_view channel);

    /// Evaluate a telemetry sample against the envelope.
    /// A violation is routed to AlertManager with id
    /// "envelope.<variable>".
    EnvelopeCheckResult evaluateSample(std::string_view variable, double value);

    /// Evaluate a command request through the CommandAuthorization policy.
    /// Denied commands are also recorded as an Advisory alert so that the
    /// operator sees the refusal. Pass-through otherwise.
    AuthDecision evaluateCommand(AuthRequest req);

    /// Request a mission phase transition. Rejected transitions raise a
    /// Caution alert (operator action required / guard failed).
    TransitionResult requestTransition(FlightPhase target,
                                       std::string_view reason = {});

    /// Advance the watchdog and promote any elevated staleness to an alert.
    /// Called on a periodic tick (typically 5 Hz).
    AlertLevel tick();

    // ─────── Outputs (observe reactions) ──────────────────────────────────

    /// Subscribe to AlertManager events (raise + ack).
    void subscribeAlerts(AlertManager::Sink s) { _alerts.subscribe(std::move(s)); }

    /// Subscribe to raw envelope check results (lossless).
    void subscribeEnvelope(FlightSafetyEnvelope::Sink s) { _envelope.subscribe(std::move(s)); }

    /// Subscribe to mission state transitions (accepted + rejected).
    void subscribeMission(MissionStateMachine::Listener l) { _mission.addListener(std::move(l)); }

    // ─────── State accessors ──────────────────────────────────────────────

    FlightPhase currentPhase() const noexcept { return _mission.current(); }
    AlertLevel  masterLevel()  const noexcept { return _alerts.masterLevel(); }
    std::size_t activeAlertCount() const noexcept { return _alerts.active().size(); }

    MissionStateMachine&      mission() noexcept { return _mission; }
    Watchdog&                 watchdog() noexcept { return _watchdog; }
    AlertManager&             alerts() noexcept { return _alerts; }
    CommandAuthorization&     authorization() noexcept { return _auth; }
    FlightSafetyEnvelope&     envelope() noexcept { return _envelope; }
    FlightTerminationService& termination() noexcept { return _fts; }

    const MissionStateMachine&      mission() const noexcept { return _mission; }
    const Watchdog&                 watchdog() const noexcept { return _watchdog; }
    const AlertManager&             alerts() const noexcept { return _alerts; }
    const CommandAuthorization&     authorization() const noexcept { return _auth; }
    const FlightSafetyEnvelope&     envelope() const noexcept { return _envelope; }
    const FlightTerminationService& termination() const noexcept { return _fts; }

private:
    Clock                    _clock;
    MissionStateMachine      _mission;
    Watchdog                 _watchdog;
    AlertManager             _alerts;
    CommandAuthorization     _auth;
    FlightSafetyEnvelope     _envelope;
    FlightTerminationService _fts;

    void wire();

    static std::string alertIdForChannel(std::string_view channel);
    static std::string alertIdForEnvelope(std::string_view variable);
    static std::string alertIdForCommand(std::string_view command);

    static AlertLevel envelopeLevel(const EnvelopeCheckResult& r);
};

} // namespace m130::safety

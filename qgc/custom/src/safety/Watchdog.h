#pragma once

#include "AlertLevel.h"
#include "FlightPhase.h"

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace m130::safety {

/// Staleness thresholds for a single watched channel.
struct StalenessThresholds {
    uint64_t advisory_ms = 500;    ///< >500ms → Advisory
    uint64_t caution_ms  = 1000;   ///< >1s → Caution
    uint64_t warning_ms  = 3000;   ///< >3s → Warning
    uint64_t emergency_ms_inflight = 10000; ///< >10s during flight → Emergency
};

/// Defaults tailored to the heartbeat channel (REQ-M130-GCS-PROT-005/006/007).
constexpr StalenessThresholds defaultHeartbeatThresholds()
{
    return { 500, 1000, 3000, 10000 };
}

/// A channel is any periodic source that we expect to see updates from.
struct ChannelState {
    std::string name;
    uint64_t last_update_ms = 0;
    bool     ever_updated   = false;
    StalenessThresholds thresholds = defaultHeartbeatThresholds();
};

/// Multi-channel watchdog (REQ-M130-GCS-SAFE-003, REQ-M130-GCS-TELE-004).
///
/// Pure C++ — testable without Qt. Clients call `tick()` at regular intervals
/// (driven by a QTimer in Qt or a test loop) and `feed()` whenever a message
/// arrives for a channel. The watchdog emits structured events the caller
/// routes to AlertManager.
class Watchdog
{
public:
    using Clock = std::function<uint64_t()>;

    struct Event {
        uint64_t    timestamp_ms = 0;
        std::string channel;
        uint64_t    age_ms       = 0;
        AlertLevel  level        = AlertLevel::None;
    };

    using EventSink = std::function<void(const Event&)>;

    Watchdog();
    explicit Watchdog(Clock clock);

    /// Register a channel for monitoring.
    void addChannel(std::string name, StalenessThresholds thresholds = defaultHeartbeatThresholds());

    /// Report that a channel just produced data.
    void feed(std::string_view channel);

    /// Evaluate all channels against current time and emit events for any
    /// that exceed thresholds. Returns the highest level observed.
    AlertLevel tick(FlightPhase current_phase);

    /// Attach a sink (can be multiple).
    void subscribe(EventSink s);

    /// Read-only snapshot of current channel state.
    const std::unordered_map<std::string, ChannelState>& channels() const noexcept { return _channels; }

private:
    Clock _clock;
    std::unordered_map<std::string, ChannelState> _channels;
    std::vector<EventSink> _sinks;

    AlertLevel classify(uint64_t age_ms, const StalenessThresholds& t, FlightPhase phase) const;
    void emit(const Event& e);
};

} // namespace m130::safety

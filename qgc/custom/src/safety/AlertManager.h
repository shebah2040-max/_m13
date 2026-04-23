#pragma once

#include "AlertLevel.h"

#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace m130::safety {

/// Unique identifier for an alert source (e.g. "heartbeat", "solver", "envelope.phi").
using AlertId = std::string;

/// A single alert instance.
struct Alert {
    uint64_t    raised_ms  = 0;
    uint64_t    acked_ms   = 0; ///< 0 when not yet acked
    AlertId     id;
    AlertLevel  level      = AlertLevel::None;
    std::string title;     ///< short, user-facing
    std::string detail;    ///< full context
    std::string ack_user;  ///< user name who acknowledged, empty if not acked
};

/// Alert system implementing ARINC 661 4-level model (REQ-M130-GCS-SAFE-005).
///
/// Properties:
/// - Bounded active queue to survive alert flood (HAZ-007 mitigation).
/// - Deduplication: raising same id twice updates the existing entry.
/// - Escalation: raising higher level replaces lower; lower does not downgrade.
/// - Acknowledgement: explicit user + time; emits to sinks.
/// - Master state: `masterLevel()` is the highest unacked level.
class AlertManager
{
public:
    using Clock = std::function<uint64_t()>;
    using Sink  = std::function<void(const Alert&, bool is_ack)>;

    AlertManager();
    explicit AlertManager(Clock clock, std::size_t max_active = 256);

    /// Raise or update an alert. Returns false if the new level does not
    /// exceed the existing level for the same id (idempotent dedup).
    bool raise(AlertId id, AlertLevel level, std::string title, std::string detail = {});

    /// Acknowledge an alert by id. Returns true if an unacked alert was acked.
    bool acknowledge(const AlertId& id, std::string user);

    /// Clear an alert entirely (as if never raised).
    void clear(const AlertId& id);

    /// Highest level among unacked alerts; None if none active.
    AlertLevel masterLevel() const noexcept;

    /// Snapshot of the active (unacked) alerts, most severe first.
    std::vector<Alert> active() const;

    /// Full audit of all alerts raised or acked.
    const std::deque<Alert>& history() const noexcept { return _history; }

    /// Subscribe to alert events (raise OR ack).
    void subscribe(Sink s);

    /// Total alerts ever raised (including dedup updates).
    std::uint64_t raisedCount() const noexcept { return _raised_count; }

private:
    Clock  _clock;
    std::size_t _max_active;
    std::unordered_map<AlertId, Alert> _active;
    std::deque<Alert>  _history;
    std::vector<Sink>  _sinks;
    std::uint64_t      _raised_count = 0;

    void publish(const Alert& a, bool is_ack);
};

} // namespace m130::safety

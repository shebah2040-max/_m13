#include "SafetyAggregator.h"

#include <chrono>
#include <string>
#include <utility>

namespace m130::safety {

namespace {
uint64_t defaultClockMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}
} // namespace

SafetyAggregator::SafetyAggregator() : SafetyAggregator(&defaultClockMs) {}

SafetyAggregator::SafetyAggregator(Clock clock)
    : _clock(std::move(clock))
    , _mission(_clock)
    , _watchdog(_clock)
    , _alerts(_clock, 256)
    , _auth(_clock)
    , _envelope()
    , _fts()
{
    wire();
}

void SafetyAggregator::installDefaults()
{
    _envelope = FlightSafetyEnvelope::createDefault();
    _auth.installDefaultPolicy();
}

void SafetyAggregator::addChannel(std::string name, StalenessThresholds t)
{
    _watchdog.addChannel(std::move(name), t);
}

void SafetyAggregator::feed(std::string_view channel)
{
    _watchdog.feed(channel);
    const std::string id = alertIdForChannel(channel);
    // Any active staleness alert for this channel clears on fresh data.
    _alerts.clear(id);
}

EnvelopeCheckResult SafetyAggregator::evaluateSample(std::string_view variable, double value)
{
    const EnvelopeCheckResult r = _envelope.check(variable, value, currentPhase());
    const std::string id = alertIdForEnvelope(r.variable);
    if (r.level == AlertLevel::None) {
        _alerts.clear(id);
    } else {
        _alerts.raise(
            id,
            r.level,
            std::string("envelope: ") + r.variable,
            std::string("value=") + std::to_string(r.value)
                + " phase=" + std::string(toString(r.phase)));
    }
    return r;
}

AuthDecision SafetyAggregator::evaluateCommand(AuthRequest req)
{
    if (req.request_time_ms == 0) {
        req.request_time_ms = _clock();
    }
    if (req.phase == FlightPhase::Unknown) {
        req.phase = currentPhase();
    }
    const AuthDecision d = _auth.evaluate(req);
    if (d.result != AuthResult::Allowed) {
        _alerts.raise(
            alertIdForCommand(req.command),
            AlertLevel::Advisory,
            std::string("command denied: ") + req.command,
            d.detail);
    }
    return d;
}

TransitionResult SafetyAggregator::requestTransition(FlightPhase target, std::string_view reason)
{
    const TransitionResult r = _mission.requestTransition(target, reason);
    if (r != TransitionResult::Accepted && r != TransitionResult::RejectedNoChange) {
        std::string detail = "from=";
        detail += toString(_mission.current());
        detail += " to=";
        detail += toString(target);
        if (!reason.empty()) {
            detail += " reason=";
            detail.append(reason);
        }
        _alerts.raise(
            std::string("mission.transition"),
            AlertLevel::Caution,
            "mission transition rejected",
            detail);
    }
    return r;
}

AlertLevel SafetyAggregator::tick()
{
    return _watchdog.tick(currentPhase());
}

std::string SafetyAggregator::alertIdForChannel(std::string_view channel)
{
    std::string out = "stale.";
    out.append(channel);
    return out;
}

std::string SafetyAggregator::alertIdForEnvelope(std::string_view variable)
{
    std::string out = "envelope.";
    out.append(variable);
    return out;
}

std::string SafetyAggregator::alertIdForCommand(std::string_view command)
{
    std::string out = "cmd.denied.";
    out.append(command);
    return out;
}

void SafetyAggregator::wire()
{
    _watchdog.subscribe([this](const Watchdog::Event& e) {
        if (e.level == AlertLevel::None) {
            _alerts.clear(alertIdForChannel(e.channel));
            return;
        }
        _alerts.raise(
            alertIdForChannel(e.channel),
            e.level,
            std::string("stale: ") + e.channel,
            std::string("age_ms=") + std::to_string(e.age_ms));
    });

    _mission.addListener([this](const TransitionRecord& r) {
        if (r.result == TransitionResult::Accepted) {
            // Clear any prior transition-rejected alert on success.
            _alerts.clear("mission.transition");
        }
    });
}

} // namespace m130::safety

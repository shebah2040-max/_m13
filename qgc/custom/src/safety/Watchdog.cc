#include "Watchdog.h"

#include <chrono>

namespace m130::safety {

namespace {
uint64_t defaultClockMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}
} // namespace

Watchdog::Watchdog() : Watchdog(&defaultClockMs) {}

Watchdog::Watchdog(Clock clock) : _clock(std::move(clock)) {}

void Watchdog::addChannel(std::string name, StalenessThresholds thresholds)
{
    ChannelState s;
    s.name = name;
    s.thresholds = thresholds;
    _channels[std::move(name)] = std::move(s);
}

void Watchdog::feed(std::string_view channel)
{
    auto it = _channels.find(std::string(channel));
    if (it == _channels.end()) {
        return;
    }
    it->second.last_update_ms = _clock();
    it->second.ever_updated   = true;
}

AlertLevel Watchdog::classify(uint64_t age_ms, const StalenessThresholds& t, FlightPhase phase) const
{
    // Emergency threshold only applies in flight.
    if (isInFlight(phase) && age_ms > t.emergency_ms_inflight) {
        return AlertLevel::Emergency;
    }
    if (age_ms > t.warning_ms)  return AlertLevel::Warning;
    if (age_ms > t.caution_ms)  return AlertLevel::Caution;
    if (age_ms > t.advisory_ms) return AlertLevel::Advisory;
    return AlertLevel::None;
}

AlertLevel Watchdog::tick(FlightPhase current_phase)
{
    const uint64_t now = _clock();
    AlertLevel highest = AlertLevel::None;

    for (auto& [name, state] : _channels) {
        if (!state.ever_updated) {
            // Before first update treat as advisory (we don't know yet).
            if (moreSevereThan(AlertLevel::Advisory, highest)) {
                highest = AlertLevel::Advisory;
            }
            continue;
        }
        const uint64_t age = now - state.last_update_ms;
        AlertLevel lvl = classify(age, state.thresholds, current_phase);
        if (lvl != AlertLevel::None) {
            Event e;
            e.timestamp_ms = now;
            e.channel      = name;
            e.age_ms       = age;
            e.level        = lvl;
            emit(e);
        }
        if (moreSevereThan(lvl, highest)) {
            highest = lvl;
        }
    }
    return highest;
}

void Watchdog::subscribe(EventSink s)
{
    _sinks.push_back(std::move(s));
}

void Watchdog::emit(const Event& e)
{
    for (const auto& s : _sinks) {
        if (s) s(e);
    }
}

} // namespace m130::safety

#include "CountdownClock.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace m130::views {

bool CountdownClock::arm(std::uint64_t target_launch_ms, std::uint64_t now_ms)
{
    if (target_launch_ms <= now_ms) return false;
    _state = CountdownState::Counting;
    _target_ms = target_launch_ms;
    _hold_start_ms = 0;
    _hold_total_ms = 0;
    return true;
}

bool CountdownClock::hold(std::uint64_t now_ms)
{
    if (_state != CountdownState::Counting) return false;
    _state = CountdownState::Hold;
    _hold_start_ms = now_ms;
    return true;
}

bool CountdownClock::resume(std::uint64_t now_ms)
{
    if (_state != CountdownState::Hold) return false;
    const std::uint64_t this_hold_ms =
        now_ms >= _hold_start_ms ? now_ms - _hold_start_ms : 0;
    _target_ms     += this_hold_ms;
    _hold_total_ms += this_hold_ms;
    _state = CountdownState::Counting;
    return true;
}

void CountdownClock::abort(std::uint64_t /*now_ms*/)
{
    _state = CountdownState::Aborted;
}

void CountdownClock::tick(std::uint64_t now_ms)
{
    if (_state == CountdownState::Counting && now_ms >= _target_ms) {
        _state = CountdownState::Launched;
    }
}

double CountdownClock::secondsToLaunch(std::uint64_t now_ms) const noexcept
{
    if (_state == CountdownState::Idle || _state == CountdownState::Aborted) {
        return 0.0;
    }
    if (_state == CountdownState::Hold) {
        // Freeze at the moment the hold started.
        const std::uint64_t t = _hold_start_ms;
        return static_cast<double>(_target_ms) / 1000.0 - static_cast<double>(t) / 1000.0;
    }
    return static_cast<double>(_target_ms) / 1000.0 - static_cast<double>(now_ms) / 1000.0;
}

std::string CountdownClock::label(std::uint64_t now_ms) const
{
    if (_state == CountdownState::Idle)    return "—";
    if (_state == CountdownState::Aborted) return "ABORTED";

    const double s = secondsToLaunch(now_ms);
    const bool before = s >= 0.0;
    const double abs_s = std::abs(s);
    const auto total  = static_cast<std::uint64_t>(abs_s);
    const auto hh = total / 3600;
    const auto mm = (total / 60) % 60;
    const auto ss = total % 60;

    char buf[48];
    std::snprintf(buf, sizeof(buf), "T%s%02llu:%02llu:%02llu",
                  before ? "-" : "+",
                  static_cast<unsigned long long>(hh),
                  static_cast<unsigned long long>(mm),
                  static_cast<unsigned long long>(ss));
    std::string out(buf);
    if (_state == CountdownState::Hold)     return std::string("HOLD at ") + out;
    if (_state == CountdownState::Launched) return std::string("LAUNCHED ") + out;
    return out;
}

} // namespace m130::views

#pragma once

#include <cstdint>
#include <string>

namespace m130::views {

enum class CountdownState : std::uint8_t {
    Idle     = 0, ///< No target set
    Counting = 1, ///< Running towards launch
    Hold     = 2, ///< Operator-requested hold-at-T
    Launched = 3, ///< T=0 passed while Counting
    Aborted  = 4,
};

/// Mission T-X countdown with hold-at-T support (MIL-STD-1472H §5.6 —
/// operator-facing timers must be stable across holds and unambiguous).
///
/// Monotonic — all timing is driven by `tick(now_ms)` so unit tests can inject
/// any clock. `hold()` / `resume()` freeze and thaw without losing elapsed
/// pre-hold time. `T = 0` means "launch now" — callers should latch the
/// Launched state to start the post-launch timeline.
class CountdownClock
{
public:
    CountdownClock() noexcept = default;

    /// Arm the countdown for a future launch time (wall clock). Must be in
    /// the future relative to @p now_ms; otherwise the call is ignored and
    /// false is returned.
    bool arm(std::uint64_t target_launch_ms, std::uint64_t now_ms);

    /// Hold — freezes the clock. Returns true if the clock was Counting.
    bool hold(std::uint64_t now_ms);

    /// Resume from hold — shifts target_launch_ms forward by the hold duration.
    bool resume(std::uint64_t now_ms);

    /// Abort — terminal; requires a re-arm.
    void abort(std::uint64_t now_ms);

    /// Advance the state machine. Call on every frame / timer tick.
    void tick(std::uint64_t now_ms);

    CountdownState state() const noexcept { return _state; }

    /// Seconds to launch (positive) or seconds since launch (negative).
    double secondsToLaunch(std::uint64_t now_ms) const noexcept;

    /// "T-00:12:34" / "T+00:00:56" / "HOLD at T-00:01:00" / "—".
    std::string label(std::uint64_t now_ms) const;

    /// Total accumulated hold time since arm (ms).
    std::uint64_t holdElapsedMs() const noexcept { return _hold_total_ms; }

    std::uint64_t targetLaunchMs() const noexcept { return _target_ms; }

private:
    CountdownState _state = CountdownState::Idle;
    std::uint64_t  _target_ms = 0;
    std::uint64_t  _hold_start_ms = 0;
    std::uint64_t  _hold_total_ms = 0;
};

} // namespace m130::views

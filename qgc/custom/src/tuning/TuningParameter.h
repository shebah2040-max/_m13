#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace m130::tuning {

// A single named tunable scalar with hard bounds, rate limit, and an
// authorisation predicate. The parameter model never clamps silently —
// out-of-range sets are rejected with a reason so the operator sees the
// failure rather than a mystery value.

struct TuningParameterSpec {
    std::string  name;
    double       min_value;
    double       max_value;
    double       default_value;
    // Max absolute delta per call. 0 disables rate limiting.
    double       max_step = 0.0;
    // Optional unit hint for UI display ("N·m", "rad²", "-", ...).
    std::string  unit;
    // Short description for help tooltips / audit log entries.
    std::string  description;
};

enum class SetResult {
    Ok,
    OutOfRange,
    RateLimited,
    SafetyGated,
    UnknownParameter,
};

inline const char* describe(SetResult r) noexcept
{
    switch (r) {
    case SetResult::Ok:                return "ok";
    case SetResult::OutOfRange:        return "out of range";
    case SetResult::RateLimited:       return "rate limited";
    case SetResult::SafetyGated:       return "safety gated";
    case SetResult::UnknownParameter:  return "unknown parameter";
    }
    return "unknown";
}

} // namespace m130::tuning

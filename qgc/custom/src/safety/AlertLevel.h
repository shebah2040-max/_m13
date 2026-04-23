#pragma once

#include <cstdint>
#include <string_view>

namespace m130::safety {

/// ARINC 661 alert level — matches mavlink m130.xml enum M130_ALERT_LEVEL.
enum class AlertLevel : uint8_t {
    None      = 0,
    Advisory  = 1,
    Caution   = 2,
    Warning   = 3,
    Emergency = 4,
};

constexpr std::string_view toString(AlertLevel l)
{
    switch (l) {
    case AlertLevel::None:      return "NONE";
    case AlertLevel::Advisory:  return "ADVISORY";
    case AlertLevel::Caution:   return "CAUTION";
    case AlertLevel::Warning:   return "WARNING";
    case AlertLevel::Emergency: return "EMERGENCY";
    }
    return "?";
}

/// Strict total order; higher numeric = more severe.
constexpr bool moreSevereThan(AlertLevel a, AlertLevel b)
{
    return static_cast<uint8_t>(a) > static_cast<uint8_t>(b);
}

} // namespace m130::safety

#pragma once

#include <cstdint>
#include <string_view>

namespace m130::safety {

/// Mission state machine phases — matches mavlink m130.xml enum M130_FLIGHT_PHASE.
enum class FlightPhase : uint8_t {
    Unknown    = 0,
    Idle       = 1,
    Prelaunch  = 2,
    Armed      = 3,
    Boost      = 4,
    Cruise     = 5,
    Terminal   = 6,
    Landed     = 7,
    Abort      = 8,
};

constexpr std::string_view toString(FlightPhase p)
{
    switch (p) {
    case FlightPhase::Unknown:   return "UNKNOWN";
    case FlightPhase::Idle:      return "IDLE";
    case FlightPhase::Prelaunch: return "PRELAUNCH";
    case FlightPhase::Armed:     return "ARMED";
    case FlightPhase::Boost:     return "BOOST";
    case FlightPhase::Cruise:    return "CRUISE";
    case FlightPhase::Terminal:  return "TERMINAL";
    case FlightPhase::Landed:    return "LANDED";
    case FlightPhase::Abort:     return "ABORT";
    }
    return "?";
}

/// Phases where flight is active (no ground operations allowed).
constexpr bool isInFlight(FlightPhase p)
{
    return p == FlightPhase::Boost
        || p == FlightPhase::Cruise
        || p == FlightPhase::Terminal;
}

/// Phases where the vehicle is on the ground and safe to service.
constexpr bool isOnGround(FlightPhase p)
{
    return p == FlightPhase::Idle
        || p == FlightPhase::Prelaunch
        || p == FlightPhase::Landed
        || p == FlightPhase::Unknown;
}

} // namespace m130::safety

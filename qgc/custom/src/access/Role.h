#pragma once

#include <cstdint>
#include <string_view>

namespace m130::access {

/// Ordered roles — later roles encompass prior privileges.
enum class Role : uint8_t {
    None            = 0,
    Observer        = 1,
    Operator        = 2,
    FlightDirector  = 3,
    SafetyOfficer   = 4,
    RangeSafety     = 5,
    Admin           = 6,
};

constexpr std::string_view toString(Role r)
{
    switch (r) {
    case Role::None:           return "NONE";
    case Role::Observer:       return "OBSERVER";
    case Role::Operator:       return "OPERATOR";
    case Role::FlightDirector: return "FLIGHT_DIRECTOR";
    case Role::SafetyOfficer:  return "SAFETY_OFFICER";
    case Role::RangeSafety:    return "RANGE_SAFETY";
    case Role::Admin:          return "ADMIN";
    }
    return "?";
}

/// True if @p granted satisfies the minimum @p required.
///
/// Roles form a linear authority lattice: the higher the numeric value, the
/// more privileged the role. RSO and SafetyOfficer are both required for
/// dual-auth commands (see FlightTerminationService).
constexpr bool satisfies(Role granted, Role required)
{
    return static_cast<uint8_t>(granted) >= static_cast<uint8_t>(required);
}

} // namespace m130::access

#pragma once

#include "../geo/Wgs84.h"
#include "MapCorridor.h"

#include <chrono>
#include <cstdint>

namespace m130::nav {

enum class CorridorState : std::uint8_t {
    Unknown = 0,
    Inside  = 1,
    Warning = 2, ///< Inside, but margin < warning threshold
    Breach  = 3, ///< Outside the corridor
};

struct CorridorConfig {
    double warn_margin_m        = 500.0;
    double clear_margin_m       = 650.0; ///< Hysteresis back to Inside
    std::chrono::milliseconds breach_persistence{200};
};

struct CorridorStatus {
    CorridorState state  = CorridorState::Unknown;
    double        margin = 0.0; ///< signed metres
    bool          just_breached = false;
    bool          just_cleared  = false;
};

/// State machine around MapCorridor that adds hysteresis and a
/// persistence timer so a single glitchy sample doesn't flap the
/// alert. Pure C++, suitable for feeding `AlertManager`.
class CorridorMonitor
{
public:
    void setCorridor(const MapCorridor* corridor) noexcept { _corridor = corridor; }
    void setConfig(const CorridorConfig& cfg) noexcept { _cfg = cfg; }
    const CorridorConfig& config() const noexcept { return _cfg; }

    CorridorStatus update(const geo::GeoPoint& p,
                          std::chrono::steady_clock::time_point now);

    CorridorState state() const noexcept { return _state; }

private:
    const MapCorridor*                    _corridor = nullptr;
    CorridorConfig                        _cfg;
    CorridorState                         _state = CorridorState::Unknown;
    std::chrono::steady_clock::time_point _first_breach_seen{};
    bool                                  _breach_pending = false;
    bool                                  _was_breach     = false;
};

} // namespace m130::nav

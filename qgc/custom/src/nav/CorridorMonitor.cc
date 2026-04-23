#include "CorridorMonitor.h"

namespace m130::nav {

CorridorStatus CorridorMonitor::update(const geo::GeoPoint& p,
                                       std::chrono::steady_clock::time_point now)
{
    CorridorStatus out;
    if (!_corridor) {
        out.state = CorridorState::Unknown;
        _state    = out.state;
        return out;
    }

    const auto margin_opt = _corridor->marginMeters(p);
    if (!margin_opt) {
        out.state = CorridorState::Unknown;
        _state    = out.state;
        return out;
    }
    const double margin = *margin_opt;
    out.margin = margin;

    CorridorState desired;
    if (margin < 0.0) {
        // Outside — need persistence before declaring Breach.
        if (!_breach_pending) {
            _breach_pending    = true;
            _first_breach_seen = now;
        }
        const auto held = now - _first_breach_seen;
        if (held >= _cfg.breach_persistence) {
            desired = CorridorState::Breach;
        } else {
            desired = (_state == CorridorState::Breach) ? CorridorState::Breach
                                                        : _state; // hold prior
            if (desired == CorridorState::Unknown) desired = CorridorState::Warning;
        }
    } else {
        _breach_pending = false;
        if (_state == CorridorState::Breach) {
            // Hysteresis: require clear_margin_m to return to Inside.
            desired = (margin >= _cfg.clear_margin_m) ? CorridorState::Inside
                                                      : CorridorState::Warning;
        } else if (margin < _cfg.warn_margin_m) {
            desired = CorridorState::Warning;
        } else {
            desired = CorridorState::Inside;
        }
    }

    out.just_breached = (_state != CorridorState::Breach) && (desired == CorridorState::Breach);
    if (desired == CorridorState::Breach) {
        _was_breach = true;
    }
    out.just_cleared = _was_breach && (desired == CorridorState::Inside);
    if (out.just_cleared) {
        _was_breach = false;
    }
    _state = desired;
    out.state = desired;
    return out;
}

} // namespace m130::nav

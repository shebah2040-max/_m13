#include "MissionStateMachine.h"

#include <chrono>

namespace m130::safety {

namespace {

/// Legal transition table. Strictly whitelist; anything not here is rejected.
///
/// Design note: ABORT is reachable from any in-flight phase unconditionally.
/// Landed is terminal; Unknown only transitions to Idle (fresh boot discovery).
struct Edge { FlightPhase from; FlightPhase to; };

constexpr Edge kLegal[] = {
    // Discovery
    { FlightPhase::Unknown,   FlightPhase::Idle      },

    // Ground prep
    { FlightPhase::Idle,      FlightPhase::Prelaunch },
    { FlightPhase::Prelaunch, FlightPhase::Idle      },  // safe downgrade
    { FlightPhase::Prelaunch, FlightPhase::Armed     },
    { FlightPhase::Armed,     FlightPhase::Prelaunch },  // disarm

    // Launch sequence
    { FlightPhase::Armed,     FlightPhase::Boost     },
    { FlightPhase::Boost,     FlightPhase::Cruise    },
    { FlightPhase::Cruise,    FlightPhase::Terminal  },
    { FlightPhase::Terminal,  FlightPhase::Landed    },

    // Abort from any active phase
    { FlightPhase::Armed,     FlightPhase::Abort     },
    { FlightPhase::Boost,     FlightPhase::Abort     },
    { FlightPhase::Cruise,    FlightPhase::Abort     },
    { FlightPhase::Terminal,  FlightPhase::Abort     },
    { FlightPhase::Abort,     FlightPhase::Landed    },
};

uint64_t defaultClockMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

} // namespace

bool MissionStateMachine::isLegal(FlightPhase from, FlightPhase to)
{
    for (const auto& e : kLegal) {
        if (e.from == from && e.to == to) {
            return true;
        }
    }
    return false;
}

MissionStateMachine::MissionStateMachine()
    : MissionStateMachine(&defaultClockMs)
{}

MissionStateMachine::MissionStateMachine(Clock clock)
    : _clock(std::move(clock))
{}

void MissionStateMachine::setGuard(FlightPhase from, FlightPhase to, Guard g)
{
    for (auto& e : _guards) {
        if (e.from == from && e.to == to) {
            e.g = std::move(g);
            return;
        }
    }
    _guards.push_back({ from, to, std::move(g) });
}

void MissionStateMachine::addListener(Listener l)
{
    _listeners.push_back(std::move(l));
}

TransitionResult MissionStateMachine::requestTransition(FlightPhase target, std::string_view reason)
{
    TransitionRecord rec;
    rec.timestamp_ms = _clock();
    rec.from = _current;
    rec.to   = target;
    rec.reason.assign(reason);

    // Early rejections.
    if (target == _current) {
        rec.result = TransitionResult::RejectedNoChange;
        notify(rec);
        _history.push_back(rec);
        return rec.result;
    }
    if (_current == FlightPhase::Landed) {
        rec.result = TransitionResult::RejectedTerminal;
        notify(rec);
        _history.push_back(rec);
        return rec.result;
    }
    if (!isLegal(_current, target)) {
        rec.result = TransitionResult::RejectedIllegal;
        notify(rec);
        _history.push_back(rec);
        return rec.result;
    }

    // Guard evaluation (except ABORT which overrides for safety).
    if (target != FlightPhase::Abort) {
        for (const auto& ge : _guards) {
            if (ge.from == _current && ge.to == target && ge.g && !ge.g(_current, target)) {
                rec.result = TransitionResult::RejectedGuard;
                notify(rec);
                _history.push_back(rec);
                return rec.result;
            }
        }
    }

    // Accept.
    rec.result = TransitionResult::Accepted;
    _current = target;
    notify(rec);
    _history.push_back(rec);
    return rec.result;
}

void MissionStateMachine::notify(const TransitionRecord& r)
{
    for (const auto& l : _listeners) {
        if (l) {
            l(r);
        }
    }
}

} // namespace m130::safety

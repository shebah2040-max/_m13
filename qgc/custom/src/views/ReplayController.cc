#include "ReplayController.h"

namespace m130::views {

bool ReplayController::loadFile(const std::string& path)
{
    const bool ok = _engine.open(path);
    _path = path;
    _state = ok ? ReplayState::Paused : ReplayState::Empty;
    if (ok && _sink) _engine.setSink(_sink);
    return ok;
}

void ReplayController::play(std::uint64_t wall_us)
{
    if (_state == ReplayState::Empty) return;
    if (_state == ReplayState::Finished) return;
    _engine.startAt(wall_us);
    _state = ReplayState::Playing;
}

void ReplayController::pause()
{
    if (_state == ReplayState::Playing) _state = ReplayState::Paused;
}

void ReplayController::setSpeed(float speed)
{
    _engine.setSpeed(speed);
}

bool ReplayController::rewind()
{
    const bool ok = _engine.rewind();
    _state = ok ? ReplayState::Paused : ReplayState::Empty;
    return ok;
}

std::size_t ReplayController::step(std::uint64_t wall_us)
{
    if (_state != ReplayState::Playing) return 0;
    const auto n = _engine.step(wall_us);
    if (_engine.isEof()) _state = ReplayState::Finished;
    return n;
}

} // namespace m130::views

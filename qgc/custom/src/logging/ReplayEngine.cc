#include "ReplayEngine.h"

#include <algorithm>

namespace m130::logging {

namespace {
constexpr float kMinSpeed = 0.1f;
constexpr float kMaxSpeed = 100.0f;
} // namespace

bool ReplayEngine::open(const std::string& path)
{
    _path = path;
    _primed = false;
    _eof = false;
    _has_pending = false;
    _emitted = 0;
    return _reader.open(path);
}

void ReplayEngine::setSpeed(float speed)
{
    if (!(speed > 0.0f)) speed = kMinSpeed;
    _speed = std::clamp(speed, kMinSpeed, kMaxSpeed);
}

void ReplayEngine::startAt(std::uint64_t wall_us)
{
    _wall_anchor_us = wall_us;
    // Peek at first frame to establish stream anchor.
    if (!_has_pending) {
        auto f = _reader.next();
        if (!f) {
            _eof = true;
            _primed = true;
            return;
        }
        _pending = std::move(*f);
        _has_pending = true;
    }
    _stream_anchor_us = _pending.timestamp_us;
    _primed = true;
}

std::size_t ReplayEngine::step(std::uint64_t wall_us)
{
    if (!_reader.isOpen() || _eof) return 0;
    if (!_primed) startAt(wall_us);
    if (_eof) return 0;

    std::size_t emitted = 0;
    while (true) {
        if (!_has_pending) {
            auto f = _reader.next();
            if (!f) { _eof = true; break; }
            _pending = std::move(*f);
            _has_pending = true;
        }

        // Scale: real_dt = (stream_ts - stream_anchor) / speed
        const std::uint64_t stream_dt_us = _pending.timestamp_us >= _stream_anchor_us
            ? _pending.timestamp_us - _stream_anchor_us
            : 0;
        const auto scaled_dt_us = static_cast<std::uint64_t>(
            static_cast<double>(stream_dt_us) / static_cast<double>(_speed));
        const std::uint64_t due_wall_us = _wall_anchor_us + scaled_dt_us;
        if (wall_us < due_wall_us) break;

        if (_sink) _sink(_pending);
        ++emitted;
        ++_emitted;
        _has_pending = false;
    }
    return emitted;
}

bool ReplayEngine::rewind()
{
    const bool ok = _reader.open(_path);
    _primed = false;
    _eof = false;
    _has_pending = false;
    return ok;
}

} // namespace m130::logging

#pragma once

#include "FdrReader.h"
#include "FlightDataRecorder.h"

#include <cstdint>
#include <functional>
#include <string>

namespace m130::logging {

/// Replay a `.m130raw` file at a configurable speed. Pure-C++ core; does not
/// block. Host drives the engine via step(current_wall_us) and consumes frames
/// through @p sink. Typical GUI flow:
///   engine.open(path);
///   engine.setSpeed(1.0f);
///   QTimer::start(10ms) → engine.step(now_us()) → ...
class ReplayEngine
{
public:
    using Sink = std::function<void(const RawFrame&)>;

    /// Open a raw stream. Returns false if the file is missing/empty.
    bool open(const std::string& path);

    /// 0.1× .. 100× bounded. Negative/zero are clamped to 0.1×.
    void setSpeed(float speed);
    float speed() const noexcept { return _speed; }

    /// Provide a sink that receives replayed frames.
    void setSink(Sink sink) { _sink = std::move(sink); }

    /// Anchor wall-clock at @p wall_us aligned to the stream's first frame
    /// timestamp. Called implicitly by step() on first call if not primed.
    void startAt(std::uint64_t wall_us);

    /// Drive the engine: emit every frame whose scaled timestamp is <= now.
    /// Returns number of frames emitted this step.
    std::size_t step(std::uint64_t wall_us);

    /// Rewind to the start of the file. Keeps current speed.
    bool rewind();

    bool isOpen() const noexcept { return _reader.isOpen(); }
    bool isEof() const noexcept { return _eof; }
    std::uint64_t framesEmitted() const noexcept { return _emitted; }

private:
    FdrRawReader  _reader;
    Sink          _sink;
    float         _speed = 1.0f;
    std::uint64_t _wall_anchor_us = 0;
    std::uint64_t _stream_anchor_us = 0;
    bool          _primed = false;
    bool          _eof = false;
    bool          _has_pending = false;
    RawFrame      _pending{};
    std::uint64_t _emitted = 0;
    std::string   _path;
};

} // namespace m130::logging

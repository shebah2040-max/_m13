#pragma once

#include "../logging/FlightDataRecorder.h"
#include "../logging/ReplayEngine.h"

#include <functional>
#include <string>

namespace m130::views {

enum class ReplayState : std::uint8_t {
    Empty    = 0, ///< No file loaded
    Paused   = 1, ///< File loaded but not playing
    Playing  = 2,
    Finished = 3, ///< EOF reached
};

/// Thin play/pause/speed wrapper around ReplayEngine. Pure C++ so it can be
/// tested without Qt; a QObject facade registered via QML will live beside
/// this class in a later PR. The controller keeps a wall-clock independent
/// of the engine so `pause()` immediately stops frame emission.
class ReplayController
{
public:
    using Sink = std::function<void(const logging::RawFrame&)>;

    ReplayController() noexcept = default;

    bool loadFile(const std::string& path);

    /// Start playback at the current wall clock. Does nothing if no file.
    void play(std::uint64_t wall_us);

    /// Freeze playback. Subsequent `step()` calls emit nothing until `play`.
    void pause();

    /// Speed change takes effect on the next frame; value is clamped by the
    /// engine itself to `[0.1, 100.0]`.
    void setSpeed(float speed);

    /// Rewind to start. Keeps the sink and the current speed.
    bool rewind();

    /// Drive forward. Returns number of frames emitted.
    std::size_t step(std::uint64_t wall_us);

    void setSink(Sink sink) { _sink = std::move(sink); _engine.setSink(_sink); }

    ReplayState state() const noexcept { return _state; }
    bool isPlaying()    const noexcept { return _state == ReplayState::Playing; }
    bool isFinished()   const noexcept { return _state == ReplayState::Finished; }
    float speed()       const noexcept { return _engine.speed(); }
    std::uint64_t framesEmitted() const noexcept { return _engine.framesEmitted(); }
    const std::string& path() const noexcept { return _path; }

private:
    logging::ReplayEngine _engine;
    Sink                  _sink;
    ReplayState           _state = ReplayState::Empty;
    std::string           _path;
};

} // namespace m130::views

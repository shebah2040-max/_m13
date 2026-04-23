#pragma once

#include <cstdint>
#include <fstream>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace m130::logging {

/// Per-message header captured in the FDR raw stream.
struct RawFrame {
    uint64_t    timestamp_us = 0;
    uint8_t     channel = 0;      ///< 0=telemetry-in, 1=command-out, 2=event
    uint16_t    msg_id = 0;
    std::vector<std::uint8_t> payload;
};

/// Event record (human-facing).
struct EventRecord {
    uint64_t    timestamp_ms = 0;
    std::string level;   ///< "info", "warn", "error", "alert"
    std::string source;  ///< e.g. "safety.watchdog"
    std::string message;
};

/// Flight Data Recorder (REQ-M130-GCS-LOG-001/002).
///
/// Foundation scope:
///   - Raw MAVLink stream → binary length-prefixed log (".m130raw")
///   - Event stream → JSON-lines log (".m130events")
///   - Parquet/Arrow + video + screenshots land in a follow-up PR
///
/// Thread-safe via internal mutex. Designed to be cheap: no allocation on hot
/// path beyond the payload copy.
class FlightDataRecorder
{
public:
    using Clock = std::function<uint64_t()>;

    FlightDataRecorder();
    explicit FlightDataRecorder(Clock clock);
    ~FlightDataRecorder();

    /// Open both files rooted at @p base_path (no extension — extensions added).
    bool open(const std::string& base_path);

    /// Close files. Safe to call multiple times.
    void close();

    bool isOpen() const noexcept;

    /// Append a raw frame to the binary log.
    void appendRaw(const RawFrame& f);

    /// Append an event to the JSONL log.
    void appendEvent(const EventRecord& e);

    /// Simple counters for observability.
    std::uint64_t rawFrameCount()  const noexcept { return _raw_count; }
    std::uint64_t eventCount()     const noexcept { return _event_count; }
    std::uint64_t bytesWritten()   const noexcept { return _bytes_written; }

    const std::string& rawPath()    const noexcept { return _raw_path; }
    const std::string& eventPath()  const noexcept { return _event_path; }

private:
    mutable std::mutex _m;
    Clock _clock;
    std::ofstream _raw;
    std::ofstream _events;
    std::string _raw_path;
    std::string _event_path;
    std::uint64_t _raw_count = 0;
    std::uint64_t _event_count = 0;
    std::uint64_t _bytes_written = 0;
};

} // namespace m130::logging

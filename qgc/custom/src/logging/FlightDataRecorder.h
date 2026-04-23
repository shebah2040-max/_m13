#pragma once

#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace m130::logging {

/// Raw MAVLink frame captured in the FDR raw stream.
struct RawFrame {
    std::uint64_t timestamp_us = 0;
    std::uint8_t  channel = 0;    ///< 0=telemetry-in, 1=command-out, 2=event
    std::uint16_t msg_id = 0;
    std::vector<std::uint8_t> payload;
};

/// Event record (human-facing).
struct EventRecord {
    std::uint64_t timestamp_ms = 0;
    std::string   level;   ///< "info", "warn", "error", "alert"
    std::string   source;  ///< e.g. "safety.watchdog"
    std::string   message;
};

/// Decoded/structured telemetry sample. Emitted per message after dialect
/// dispatch so post-flight analysis can work without reparsing raw payloads.
struct StructuredSample {
    std::uint64_t timestamp_us = 0;
    std::uint32_t msg_id = 0;
    std::string   msg_name;
    /// Ordered (key, string-serialised value). Numeric values are written
    /// unquoted in JSON; other values are quoted. Keep insertion order stable
    /// so post-flight tooling can rely on it.
    std::vector<std::pair<std::string, std::string>> numeric;
    std::vector<std::pair<std::string, std::string>> text;
};

/// Flight Data Recorder (REQ-M130-GCS-LOG-001 / -002 / -007).
///
/// Three parallel tracks on disk (all rooted at a single base path):
///   - `<base>.m130raw`        — binary, length-prefixed raw MAVLink frames
///   - `<base>.m130events`     — JSON-lines, operator/safety events
///   - `<base>.m130structured` — JSON-lines, decoded dialect samples
/// On close, a session manifest (`<base>.m130session.json`) is written that
/// records counters and per-track content hashes. Pair with ChainOfCustody to
/// sign the manifest.
///
/// Thread-safe via internal mutex. Hot path keeps allocations bounded.
class FlightDataRecorder
{
public:
    using Clock = std::function<std::uint64_t()>;

    FlightDataRecorder();
    explicit FlightDataRecorder(Clock clock_us);
    ~FlightDataRecorder();

    /// Open the three tracks rooted at @p base_path (no extension — extensions
    /// are appended automatically). Returns true if all files were opened.
    bool open(const std::string& base_path);

    /// Flush and close. Writes the session manifest as a side-effect. Safe to
    /// call multiple times.
    void close();

    bool isOpen() const noexcept;

    void appendRaw(const RawFrame& f);
    void appendEvent(const EventRecord& e);
    void appendStructured(const StructuredSample& s);

    std::uint64_t rawFrameCount()     const noexcept { return _raw_count; }
    std::uint64_t eventCount()        const noexcept { return _event_count; }
    std::uint64_t structuredCount()   const noexcept { return _structured_count; }
    std::uint64_t bytesWritten()      const noexcept { return _bytes_written; }
    std::uint64_t sessionStartUs()    const noexcept { return _session_start_us; }
    std::uint64_t sessionEndUs()      const noexcept { return _session_end_us; }

    const std::string& rawPath()        const noexcept { return _raw_path; }
    const std::string& eventPath()      const noexcept { return _event_path; }
    const std::string& structuredPath() const noexcept { return _structured_path; }
    const std::string& manifestPath()   const noexcept { return _manifest_path; }

private:
    void _writeManifest();

    mutable std::mutex _m;
    Clock         _clock;
    std::ofstream _raw;
    std::ofstream _events;
    std::ofstream _structured;
    std::string   _raw_path;
    std::string   _event_path;
    std::string   _structured_path;
    std::string   _manifest_path;
    std::uint64_t _raw_count = 0;
    std::uint64_t _event_count = 0;
    std::uint64_t _structured_count = 0;
    std::uint64_t _bytes_written = 0;
    std::uint64_t _session_start_us = 0;
    std::uint64_t _session_end_us = 0;
};

} // namespace m130::logging

#include "FlightDataRecorder.h"

#include "Sha256.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace m130::logging {

namespace {

std::uint64_t defaultClockUs()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count());
}

std::string jsonEscape(std::string_view s)
{
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x",
                              static_cast<unsigned>(static_cast<unsigned char>(c)));
                out += buf;
            } else {
                out.push_back(c);
            }
        }
    }
    return out;
}

std::string hashFile(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return {};
    crypto::Sha256 h;
    std::vector<char> buf(64 * 1024);
    while (in) {
        in.read(buf.data(), static_cast<std::streamsize>(buf.size()));
        const std::streamsize got = in.gcount();
        if (got > 0) h.update(buf.data(), static_cast<std::size_t>(got));
    }
    return crypto::Sha256::toHex(h.finalize());
}

std::uint64_t fileSize(const std::string& path)
{
    std::error_code ec;
    const auto sz = std::filesystem::file_size(path, ec);
    return ec ? 0u : sz;
}

} // namespace

FlightDataRecorder::FlightDataRecorder() : FlightDataRecorder(&defaultClockUs) {}

FlightDataRecorder::FlightDataRecorder(Clock clock_us) : _clock(std::move(clock_us)) {}

FlightDataRecorder::~FlightDataRecorder()
{
    close();
}

bool FlightDataRecorder::open(const std::string& base_path)
{
    std::lock_guard lk(_m);
    if (_raw.is_open() || _events.is_open() || _structured.is_open()) {
        // Manifest requires a lock-less finalize; release and close directly.
        _raw.flush();        _raw.close();
        _events.flush();     _events.close();
        _structured.flush(); _structured.close();
    }

    _raw_path        = base_path + ".m130raw";
    _event_path      = base_path + ".m130events";
    _structured_path = base_path + ".m130structured";
    _manifest_path   = base_path + ".m130session.json";

    _raw.open(_raw_path,               std::ios::binary | std::ios::app);
    _events.open(_event_path,          std::ios::app);
    _structured.open(_structured_path, std::ios::app);

    _raw_count = _event_count = _structured_count = 0;
    _bytes_written = 0;
    _session_start_us = _clock ? _clock() : 0;
    _session_end_us = 0;

    return _raw.is_open() && _events.is_open() && _structured.is_open();
}

void FlightDataRecorder::close()
{
    bool wrote_anything = false;
    {
        std::lock_guard lk(_m);
        wrote_anything = _raw.is_open() || _events.is_open() || _structured.is_open();
        if (_raw.is_open())        { _raw.flush();        _raw.close(); }
        if (_events.is_open())     { _events.flush();     _events.close(); }
        if (_structured.is_open()) { _structured.flush(); _structured.close(); }
        if (wrote_anything && _clock) {
            _session_end_us = _clock();
        }
    }
    if (wrote_anything && !_manifest_path.empty()) {
        _writeManifest();
    }
}

bool FlightDataRecorder::isOpen() const noexcept
{
    std::lock_guard lk(_m);
    return _raw.is_open() && _events.is_open() && _structured.is_open();
}

void FlightDataRecorder::appendRaw(const RawFrame& f)
{
    std::lock_guard lk(_m);
    if (!_raw.is_open()) return;

    const std::uint64_t ts = f.timestamp_us != 0 ? f.timestamp_us : (_clock ? _clock() : 0);
    const std::uint32_t len = static_cast<std::uint32_t>(f.payload.size());

    // Binary record: uint64 ts | uint8 ch | uint16 msg_id | uint32 len | payload
    _raw.write(reinterpret_cast<const char*>(&ts), sizeof(ts));
    _raw.write(reinterpret_cast<const char*>(&f.channel), sizeof(f.channel));
    _raw.write(reinterpret_cast<const char*>(&f.msg_id), sizeof(f.msg_id));
    _raw.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) {
        _raw.write(reinterpret_cast<const char*>(f.payload.data()), len);
    }
    _bytes_written += sizeof(ts) + sizeof(f.channel) + sizeof(f.msg_id) + sizeof(len) + len;
    ++_raw_count;
}

void FlightDataRecorder::appendEvent(const EventRecord& e)
{
    std::lock_guard lk(_m);
    if (!_events.is_open()) return;

    _events << R"({"ts":)" << e.timestamp_ms
            << R"(,"level":")" << jsonEscape(e.level)
            << R"(","source":")" << jsonEscape(e.source)
            << R"(","message":")" << jsonEscape(e.message)
            << "\"}\n";
    _events.flush();
    ++_event_count;
}

void FlightDataRecorder::appendStructured(const StructuredSample& s)
{
    std::lock_guard lk(_m);
    if (!_structured.is_open()) return;

    _structured << R"({"ts_us":)" << s.timestamp_us
                << R"(,"msg_id":)" << s.msg_id
                << R"(,"msg":")" << jsonEscape(s.msg_name)
                << R"(","n":{)";
    bool first = true;
    for (const auto& [k, v] : s.numeric) {
        if (!first) _structured << ',';
        first = false;
        _structured << '"' << jsonEscape(k) << "\":" << v;
    }
    _structured << R"(},"s":{)";
    first = true;
    for (const auto& [k, v] : s.text) {
        if (!first) _structured << ',';
        first = false;
        _structured << '"' << jsonEscape(k) << "\":\"" << jsonEscape(v) << '"';
    }
    _structured << "}}\n";
    _structured.flush();
    ++_structured_count;
}

void FlightDataRecorder::_writeManifest()
{
    std::ofstream mf(_manifest_path, std::ios::trunc);
    if (!mf.is_open()) return;

    const std::string raw_hash        = hashFile(_raw_path);
    const std::string event_hash      = hashFile(_event_path);
    const std::string structured_hash = hashFile(_structured_path);

    mf << "{\n"
       << R"(  "schema_version": "1.0",)" "\n"
       << R"(  "session_start_us": )" << _session_start_us << ",\n"
       << R"(  "session_end_us": )"   << _session_end_us   << ",\n"
       << R"(  "counts": {"raw": )"   << _raw_count
       <<      R"(, "events": )"      << _event_count
       <<      R"(, "structured": )"  << _structured_count << "},\n"
       << R"(  "bytes_written": )"    << _bytes_written    << ",\n"
       << R"(  "tracks": {)" "\n"
       << R"(    "raw":        {"path": ")" << jsonEscape(_raw_path)
       <<             R"(", "size": )"     << fileSize(_raw_path)
       <<             R"(, "sha256": ")"   << raw_hash        << "\"},\n"
       << R"(    "events":     {"path": ")" << jsonEscape(_event_path)
       <<             R"(", "size": )"     << fileSize(_event_path)
       <<             R"(, "sha256": ")"   << event_hash      << "\"},\n"
       << R"(    "structured": {"path": ")" << jsonEscape(_structured_path)
       <<             R"(", "size": )"     << fileSize(_structured_path)
       <<             R"(, "sha256": ")"   << structured_hash << "\"}\n"
       << "  }\n}\n";
}

} // namespace m130::logging

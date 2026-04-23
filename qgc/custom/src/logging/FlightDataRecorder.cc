#include "FlightDataRecorder.h"

#include <chrono>
#include <cstdint>

namespace m130::logging {

namespace {
uint64_t defaultClockUs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count());
}
} // namespace

FlightDataRecorder::FlightDataRecorder() : FlightDataRecorder(&defaultClockUs) {}

FlightDataRecorder::FlightDataRecorder(Clock clock) : _clock(std::move(clock)) {}

FlightDataRecorder::~FlightDataRecorder()
{
    close();
}

bool FlightDataRecorder::open(const std::string& base_path)
{
    std::lock_guard lk(_m);
    close();
    _raw_path   = base_path + ".m130raw";
    _event_path = base_path + ".m130events";
    _raw.open(_raw_path, std::ios::binary | std::ios::app);
    _events.open(_event_path, std::ios::app);
    return _raw.is_open() && _events.is_open();
}

void FlightDataRecorder::close()
{
    if (_raw.is_open()) {
        _raw.flush();
        _raw.close();
    }
    if (_events.is_open()) {
        _events.flush();
        _events.close();
    }
}

bool FlightDataRecorder::isOpen() const noexcept
{
    std::lock_guard lk(_m);
    return _raw.is_open() && _events.is_open();
}

void FlightDataRecorder::appendRaw(const RawFrame& f)
{
    std::lock_guard lk(_m);
    if (!_raw.is_open()) return;

    const uint64_t ts = f.timestamp_us != 0 ? f.timestamp_us : _clock();
    const uint32_t len = static_cast<uint32_t>(f.payload.size());

    // Simple framed binary record:
    //  uint64 ts | uint8 ch | uint16 msg_id | uint32 len | payload
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

    const auto esc = [](const std::string& s) {
        std::string out;
        out.reserve(s.size() + 2);
        for (char c : s) {
            switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out.push_back(c);
            }
        }
        return out;
    };

    _events << R"({"ts":)" << e.timestamp_ms
            << R"(,"level":")" << esc(e.level)
            << R"(","source":")" << esc(e.source)
            << R"(","message":")" << esc(e.message)
            << "\"}\n";
    _events.flush();
    ++_event_count;
}

} // namespace m130::logging

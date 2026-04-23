#include "FdrReader.h"

#include <cstring>
#include <string>

namespace m130::logging {

bool FdrRawReader::open(const std::string& path)
{
    if (_in.is_open()) _in.close();
    _read = 0;
    _in.open(path, std::ios::binary);
    return _in.is_open() && _in.peek() != std::char_traits<char>::eof();
}

std::optional<RawFrame> FdrRawReader::next()
{
    if (!_in.is_open()) return std::nullopt;

    RawFrame f;
    std::uint32_t len = 0;

    _in.read(reinterpret_cast<char*>(&f.timestamp_us), sizeof(f.timestamp_us));
    if (_in.gcount() != static_cast<std::streamsize>(sizeof(f.timestamp_us))) return std::nullopt;
    _in.read(reinterpret_cast<char*>(&f.channel), sizeof(f.channel));
    if (_in.gcount() != 1) return std::nullopt;
    _in.read(reinterpret_cast<char*>(&f.msg_id), sizeof(f.msg_id));
    if (_in.gcount() != 2) return std::nullopt;
    _in.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (_in.gcount() != 4) return std::nullopt;

    f.payload.resize(len);
    if (len > 0) {
        _in.read(reinterpret_cast<char*>(f.payload.data()), len);
        if (static_cast<std::uint32_t>(_in.gcount()) != len) return std::nullopt;
    }
    ++_read;
    return f;
}

std::vector<RawFrame> FdrRawReader::readAll(std::size_t max)
{
    std::vector<RawFrame> out;
    while (true) {
        auto f = next();
        if (!f) break;
        out.push_back(std::move(*f));
        if (max != 0 && out.size() >= max) break;
    }
    return out;
}

// ---- events reader ---------------------------------------------------------

namespace {

// Minimal JSONL extractor for the small, well-defined shape written by FDR:
//   {"ts":<int>,"level":"...","source":"...","message":"..."}
// Not a general JSON parser; refuses anything it doesn't recognise.
bool extractString(std::string_view line, std::string_view key, std::string& out)
{
    const std::string needle = std::string("\"") + std::string(key) + "\":\"";
    const auto pos = line.find(needle);
    if (pos == std::string_view::npos) return false;
    const auto start = pos + needle.size();
    std::string result;
    for (std::size_t i = start; i < line.size(); ++i) {
        const char c = line[i];
        if (c == '\\' && i + 1 < line.size()) {
            const char nx = line[i + 1];
            switch (nx) {
            case 'n':  result.push_back('\n'); break;
            case 'r':  result.push_back('\r'); break;
            case 't':  result.push_back('\t'); break;
            case '"':  result.push_back('"');  break;
            case '\\': result.push_back('\\'); break;
            default:   result.push_back(nx);
            }
            ++i;
        } else if (c == '"') {
            out = std::move(result);
            return true;
        } else {
            result.push_back(c);
        }
    }
    return false;
}

bool extractUint(std::string_view line, std::string_view key, std::uint64_t& out)
{
    const std::string needle = std::string("\"") + std::string(key) + "\":";
    const auto pos = line.find(needle);
    if (pos == std::string_view::npos) return false;
    std::size_t i = pos + needle.size();
    std::uint64_t v = 0;
    bool any = false;
    while (i < line.size() && line[i] >= '0' && line[i] <= '9') {
        v = v * 10u + static_cast<std::uint64_t>(line[i] - '0');
        ++i;
        any = true;
    }
    if (!any) return false;
    out = v;
    return true;
}

} // namespace

bool FdrEventReader::open(const std::string& path)
{
    if (_in.is_open()) _in.close();
    _in.open(path);
    return _in.is_open();
}

std::optional<EventRecord> FdrEventReader::next()
{
    if (!_in.is_open()) return std::nullopt;
    std::string line;
    while (std::getline(_in, line)) {
        if (line.empty()) continue;
        EventRecord e;
        if (!extractUint(line, "ts", e.timestamp_ms)) continue;
        extractString(line, "level",   e.level);
        extractString(line, "source",  e.source);
        extractString(line, "message", e.message);
        return e;
    }
    return std::nullopt;
}

std::vector<EventRecord> FdrEventReader::readAll()
{
    std::vector<EventRecord> out;
    while (true) {
        auto e = next();
        if (!e) break;
        out.push_back(std::move(*e));
    }
    return out;
}

} // namespace m130::logging

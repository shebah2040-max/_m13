#include "AuditLogger.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace m130::logging {

namespace {

/// Cheap FNV-1a 64-bit hash used as default integrity function.
///
/// This is NOT cryptographic. It serves to detect ACCIDENTAL tampering and
/// establish the chain API. Production builds override it with HMAC-SHA256
/// once libsodium is wired in a follow-up PR.
uint64_t fnv1a64(std::string_view data)
{
    uint64_t h = 0xcbf29ce484222325ULL;
    for (unsigned char c : data) {
        h ^= c;
        h *= 0x100000001b3ULL;
    }
    return h;
}

std::string toHex64(uint64_t x)
{
    std::ostringstream os;
    os << std::hex << std::setw(16) << std::setfill('0') << x;
    return os.str();
}

uint64_t now_ms()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

} // namespace

AuditLogger::~AuditLogger()
{
    if (_stream.is_open()) {
        _stream.flush();
        _stream.close();
    }
}

bool AuditLogger::open(const std::string& path)
{
    std::lock_guard lk(_m);
    if (_stream.is_open()) {
        _stream.close();
    }
    _path = path;
    _stream.open(path, std::ios::app | std::ios::binary);
    return _stream.is_open();
}

void AuditLogger::setHmacKey(std::string key)
{
    std::lock_guard lk(_m);
    _key = std::move(key);
}

std::string AuditLogger::escape(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\\' || c == '|' || c == '\n' || c == '\r') {
            out.push_back('\\');
            switch (c) {
            case '\n': out.push_back('n'); break;
            case '\r': out.push_back('r'); break;
            default:   out.push_back(c);
            }
        } else {
            out.push_back(c);
        }
    }
    return out;
}

std::string AuditLogger::toLine(const AuditEntry& e)
{
    std::ostringstream os;
    os << e.timestamp_ms << '|'
       << escape(e.category) << '|'
       << escape(e.actor) << '|'
       << escape(e.action) << '|'
       << escape(e.target) << '|'
       << escape(e.detail) << '|'
       << e.prev_hash << '|'
       << e.this_hash;
    return os.str();
}

std::string AuditLogger::computeHash(const AuditEntry& e) const
{
    std::string payload;
    payload.reserve(128 + e.detail.size());
    payload.append(std::to_string(e.timestamp_ms));
    payload.push_back('|'); payload.append(e.category);
    payload.push_back('|'); payload.append(e.actor);
    payload.push_back('|'); payload.append(e.action);
    payload.push_back('|'); payload.append(e.target);
    payload.push_back('|'); payload.append(e.detail);
    payload.push_back('|'); payload.append(e.prev_hash);
    if (!_key.empty()) {
        // Poor-man HMAC fold: key XOR then hash (NOT real HMAC-SHA256 yet).
        std::string k = _key;
        for (std::size_t i = 0; i < payload.size(); ++i) {
            payload[i] = static_cast<char>(payload[i] ^ k[i % k.size()]);
        }
    }
    return toHex64(fnv1a64(payload));
}

AuditEntry AuditLogger::append(std::string category,
                               std::string actor,
                               std::string action,
                               std::string target,
                               std::string detail)
{
    std::lock_guard lk(_m);
    AuditEntry e;
    e.timestamp_ms = now_ms();
    e.category = std::move(category);
    e.actor    = std::move(actor);
    e.action   = std::move(action);
    e.target   = std::move(target);
    e.detail   = std::move(detail);
    e.prev_hash = _last_hash;
    e.this_hash = computeHash(e);

    if (_stream.is_open()) {
        _stream << toLine(e) << '\n';
        _stream.flush();
    }
    _last_hash = e.this_hash;

    _recent.push_back(e);
    if (_recent.size() > kRecentCap) {
        _recent.erase(_recent.begin(),
                      _recent.begin() + static_cast<long>(_recent.size() - kRecentCap));
    }
    ++_written;
    return e;
}

bool AuditLogger::verifyFile(const std::string& path, std::size_t* bad_line) const
{
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        if (bad_line) *bad_line = 0;
        return false;
    }
    std::string line;
    std::size_t lineno = 0;
    std::string prev_hash;
    while (std::getline(in, line)) {
        ++lineno;
        if (line.empty()) continue;

        // Parse 8 '|'-separated fields.
        std::array<std::string, 8> f;
        std::size_t slot = 0;
        std::string cur;
        bool escape_next = false;
        for (char c : line) {
            if (escape_next) {
                switch (c) {
                case 'n': cur.push_back('\n'); break;
                case 'r': cur.push_back('\r'); break;
                default:  cur.push_back(c);
                }
                escape_next = false;
            } else if (c == '\\') {
                escape_next = true;
            } else if (c == '|' && slot < f.size() - 1) {
                f[slot++] = std::move(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        f[slot] = std::move(cur);
        if (slot != f.size() - 1) {
            if (bad_line) *bad_line = lineno;
            return false;
        }
        AuditEntry e;
        try {
            e.timestamp_ms = std::stoull(f[0]);
        } catch (...) {
            if (bad_line) *bad_line = lineno;
            return false;
        }
        e.category = f[1];
        e.actor    = f[2];
        e.action   = f[3];
        e.target   = f[4];
        e.detail   = f[5];
        e.prev_hash= f[6];
        const std::string& recorded = f[7];

        if (e.prev_hash != prev_hash) {
            if (bad_line) *bad_line = lineno;
            return false;
        }
        if (computeHash(e) != recorded) {
            if (bad_line) *bad_line = lineno;
            return false;
        }
        prev_hash = recorded;
    }
    return true;
}

} // namespace m130::logging

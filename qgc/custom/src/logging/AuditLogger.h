#pragma once

#include <cstdint>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace m130::logging {

/// A single audit record. Append-only, hash-chained on disk.
struct AuditEntry {
    uint64_t    timestamp_ms = 0;
    std::string category;   ///< "command", "alert", "session", "config"
    std::string actor;      ///< user id or "system"
    std::string action;     ///< e.g. "ARM", "RAISE", "LOGIN"
    std::string target;     ///< object of action (may be empty)
    std::string detail;     ///< JSON-escaped free-form
    std::string prev_hash;  ///< hex of previous entry's hash (32 chars for hmac-sha256 truncation)
    std::string this_hash;  ///< hex of THIS entry's hash (filled on append)
};

/// Append-only audit logger with a linked hash chain (REQ-M130-GCS-LOG-004,
/// REQ-M130-GCS-ACC-002, REQ-M130-GCS-CMD-005).
///
/// Implementation notes:
/// - Hash function: a deterministic fold over "ts|cat|actor|action|target|detail|prev_hash"
///   using HMAC-SHA256 when a key is set, FNV-1a fallback otherwise.
/// - The file is written append-only (no rewrites); verifying a chain just
///   walks forward recomputing each hash.
/// - Thread-safe via an internal mutex.
///
/// Real cryptographic HMAC-SHA256 comes in a follow-up PR via libsodium; this
/// baseline keeps the API shape and test coverage stable.
class AuditLogger
{
public:
    AuditLogger() = default;
    ~AuditLogger();

    /// Open (or create) a log file. Subsequent appends write to it.
    bool open(const std::string& path);

    /// Set a symmetric HMAC key (ASCII or binary). When set, hashing uses HMAC.
    void setHmacKey(std::string key);

    /// Append an entry. Fills in timestamp + prev_hash + this_hash, returns a
    /// copy of the entry as stored.
    AuditEntry append(std::string category,
                      std::string actor,
                      std::string action,
                      std::string target = {},
                      std::string detail = {});

    /// Verify the chain of an existing log file. Returns true on success.
    /// On failure, @c bad_line (1-based) is set to the first bad line.
    bool verifyFile(const std::string& path, std::size_t* bad_line = nullptr) const;

    /// In-memory entries (the tail kept for quick queries). Bounded.
    const std::vector<AuditEntry>& recent() const noexcept { return _recent; }

    /// Current hash (hex of the last entry's this_hash).
    const std::string& lastHash() const noexcept { return _last_hash; }

    std::size_t entriesWritten() const noexcept { return _written; }

private:
    mutable std::mutex _m;
    std::ofstream _stream;
    std::string _path;
    std::string _key;
    std::string _last_hash;
    std::vector<AuditEntry> _recent;
    std::size_t _written = 0;
    static constexpr std::size_t kRecentCap = 4096;

    std::string computeHash(const AuditEntry& e) const;
    static std::string toLine(const AuditEntry& e);
    static std::string escape(std::string_view s);
};

} // namespace m130::logging

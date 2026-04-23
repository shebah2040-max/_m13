#pragma once

#include "Role.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace m130::access {

struct Session {
    std::string session_id;
    std::string user_id;
    Role        role = Role::None;
    uint64_t    issued_at_ms = 0;
    uint64_t    last_active_ms = 0;
    uint64_t    expires_at_ms = 0;
    bool        reauth_required_for_critical = true;
};

enum class SessionState : uint8_t {
    Valid      = 0,
    NotFound   = 1,
    Expired    = 2,
    Revoked    = 3,
};

/// Session store (REQ-M130-GCS-SEC-004/005, REQ-M130-GCS-ACC-002/003).
///
/// Foundation scope: in-memory; a persistent encrypted store lands later.
class SessionManager
{
public:
    using Clock = std::function<uint64_t()>;
    using IdGenerator = std::function<std::string()>;

    SessionManager();
    SessionManager(Clock clock, IdGenerator id_gen);

    /// Default TTL if not specified per-call.
    void setDefaultTtlMs(uint64_t ttl) { _default_ttl_ms = ttl; }

    /// Create a new session. Returns the session id.
    std::string create(std::string user_id, Role role, std::optional<uint64_t> ttl_ms = std::nullopt);

    /// Validate + refresh activity time. Returns the current session or nullopt.
    std::optional<Session> touch(const std::string& session_id);

    /// Get current state of a session.
    SessionState state(const std::string& session_id) const;

    /// Revoke a session.
    void revoke(const std::string& session_id);

    /// Garbage-collect expired sessions. Returns number evicted.
    std::size_t evictExpired();

    /// Count active (non-expired, non-revoked) sessions for a user.
    std::size_t countActiveForUser(const std::string& user_id) const;

    std::size_t size() const noexcept { return _sessions.size(); }

private:
    Clock _clock;
    IdGenerator _id_gen;
    uint64_t _default_ttl_ms = 8 * 60 * 60 * 1000; // 8h
    std::unordered_map<std::string, Session> _sessions;
    std::unordered_map<std::string, bool> _revoked;

    bool isExpiredLocked(const Session& s, uint64_t now) const;
};

} // namespace m130::access

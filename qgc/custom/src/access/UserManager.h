#pragma once

#include "Role.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace m130::access {

struct UserRecord {
    std::string user_id;
    std::string display_name;
    Role role = Role::Observer;
    std::string password_hash;   ///< argon2id in production; fnv+salt in Foundation
    std::string salt;
    std::string totp_secret;      ///< base32 in production
    uint32_t    failed_attempts = 0;
    bool        locked = false;
};

enum class LoginResult : uint8_t {
    Success          = 0,
    UnknownUser      = 1,
    BadPassword      = 2,
    AccountLocked    = 3,
    RequiresTotp     = 4,
    BadTotp          = 5,
};

struct LoginOutcome {
    LoginResult result = LoginResult::Success;
    std::string detail;
};

/// User / credential store (REQ-M130-GCS-SEC-001/003, REQ-M130-GCS-ACC-004).
///
/// Foundation scope:
/// - Local users only; LDAP/AD and TLS/HSM land in a follow-up PR.
/// - Passwords hashed with fnv+salt as a placeholder. API preserves a
///   replaceable `Hasher` so upgrading to argon2id does not break callers.
/// - TOTP verification is plug-in via `TotpChecker`.
/// - Account lockout after `_lockout_threshold` failures.
class UserManager
{
public:
    using Hasher = std::function<std::string(std::string_view pw, std::string_view salt)>;
    using TotpChecker = std::function<bool(std::string_view secret, std::string_view code)>;

    UserManager();
    UserManager(Hasher h, TotpChecker t);

    /// Create (or update) a user record. Returns false if the user exists and
    /// @p overwrite is false.
    bool upsertUser(UserRecord r, bool overwrite = false);

    /// Change password for an existing user (hashes with current `Hasher`).
    bool setPassword(const std::string& user_id, std::string_view password);

    /// Configure lockout policy.
    void setLockoutThreshold(uint32_t n) { _lockout_threshold = n; }

    /// Lookup a user (read-only).
    std::optional<UserRecord> find(const std::string& user_id) const;

    /// Attempt password login. Does NOT establish a session — caller hands the
    /// result to `SessionManager`. On success the user's counters reset.
    LoginOutcome attemptPassword(const std::string& user_id, std::string_view password);

    /// Verify a TOTP code. Used by FlightTerminationService.
    bool verifyTotp(std::string_view user_id, std::string_view code) const;

    /// Manually lock/unlock (Admin).
    void setLocked(const std::string& user_id, bool locked);

    /// Observability.
    std::size_t userCount() const noexcept { return _users.size(); }

private:
    Hasher _hash;
    TotpChecker _totp;
    uint32_t _lockout_threshold = 5;
    std::unordered_map<std::string, UserRecord> _users;

    static std::string defaultHash(std::string_view pw, std::string_view salt);
};

} // namespace m130::access

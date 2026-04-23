#pragma once

#include "PasswordHasher.h"
#include "Role.h"

#include <cstdint>
#include <functional>
#include <memory>
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
    std::string password_encoded;   ///< Full self-describing hash (see IPasswordHasher)
    std::string salt;               ///< Raw salt used for the hash
    std::string totp_secret;        ///< Raw shared secret (base32 at the UI layer)
    std::uint32_t failed_attempts = 0;
    std::uint64_t lockout_started_ms = 0;
    bool locked = false;
};

enum class LoginResult : std::uint8_t {
    Success          = 0,
    UnknownUser      = 1,
    BadPassword      = 2,
    AccountLocked    = 3,
    RequiresTotp     = 4,
    BadTotp          = 5,
    WeakPassword     = 6,
};

struct LoginOutcome {
    LoginResult result = LoginResult::Success;
    std::string detail;
};

struct LockoutPolicy {
    std::uint32_t threshold = 5;
    std::uint64_t duration_ms = 15 * 60 * 1000; ///< 15-minute lockout (NIST 800-63B §5.2.2)
};

struct PasswordPolicy {
    std::size_t min_length = 12;
    bool        require_upper  = true;
    bool        require_lower  = true;
    bool        require_digit  = true;
    bool        require_symbol = true;

    /// True when @p pw satisfies every configured requirement. `reason` is
    /// populated with the first violation.
    bool isValid(std::string_view pw, std::string* reason = nullptr) const;
};

/// User / credential store (REQ-M130-GCS-SEC-001/003, REQ-M130-GCS-ACC-004).
///
/// Pillar 6:
///   * Default hasher is PBKDF2-HMAC-SHA-256 (OWASP 2023 iteration count).
///   * Lockout is *time-based*: after `threshold` failures the account
///     locks for `duration_ms`; further attempts before that elapses keep
///     it locked. `attemptPassword` auto-unlocks once the duration passes.
///   * `PasswordPolicy` rejects trivially weak passwords on `setPassword`.
///   * TOTP verification is pluggable via `TotpChecker`; the default
///     checker uses `crypto::Totp::verify` against a real clock.
class UserManager
{
public:
    using Clock = std::function<std::uint64_t()>; ///< ms since epoch
    using TotpChecker = std::function<bool(std::string_view secret,
                                           std::string_view code,
                                           std::uint64_t now_s)>;

    UserManager();
    UserManager(std::shared_ptr<IPasswordHasher> hasher,
                TotpChecker totp = {},
                Clock clock = {});

    void setLockoutPolicy(LockoutPolicy p) noexcept { _lockout = p; }
    void setPasswordPolicy(PasswordPolicy p) noexcept { _policy = p; }
    const LockoutPolicy&  lockoutPolicy()  const noexcept { return _lockout; }
    const PasswordPolicy& passwordPolicy() const noexcept { return _policy; }

    /// Create or update a user record. Returns false if the user exists and
    /// @p overwrite is false.
    bool upsertUser(UserRecord r, bool overwrite = false);

    /// Hash and store a password. Enforces `PasswordPolicy`. If @p salt is
    /// empty, generates one from a monotonic seed. Returns false on unknown
    /// user or policy violation.
    bool setPassword(const std::string& user_id, std::string_view password,
                     std::string_view salt = {});

    /// Lookup a user (read-only).
    std::optional<UserRecord> find(const std::string& user_id) const;

    /// Attempt password login. Does NOT establish a session — caller hands
    /// the result to `SessionManager`. Advances `failed_attempts` on wrong
    /// password, auto-unlocks after `duration_ms`, resets counters on
    /// success.
    LoginOutcome attemptPassword(const std::string& user_id, std::string_view password);

    /// Verify a TOTP code against the user's stored secret. Returns false
    /// for unknown users, users without a secret, or failed verification.
    bool verifyTotp(std::string_view user_id, std::string_view code) const;

    /// Manually lock or unlock an account (admin action).
    void setLocked(const std::string& user_id, bool locked);

    /// Observability.
    std::size_t userCount()   const noexcept { return _users.size(); }
    std::uint64_t nowMs()     const         { return _clock ? _clock() : 0; }

private:
    std::shared_ptr<IPasswordHasher> _hasher;
    TotpChecker _totp;
    Clock _clock;
    LockoutPolicy _lockout;
    PasswordPolicy _policy;
    std::unordered_map<std::string, UserRecord> _users;
    std::uint64_t _salt_seed = 0x9E3779B97F4A7C15ULL;

    std::string _generateSalt();
    bool        _isCurrentlyLocked(UserRecord& r, std::uint64_t now_ms) const;
};

} // namespace m130::access

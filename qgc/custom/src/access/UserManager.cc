#include "UserManager.h"

#include "Totp.h"

#include <cctype>

namespace m130::access {

namespace {
bool hasChar(std::string_view s, int (*pred)(int))
{
    for (unsigned char c : s) if (pred(c)) return true;
    return false;
}

bool hasSymbol(std::string_view s)
{
    for (unsigned char c : s) {
        if (!std::isalnum(c) && !std::isspace(c)) return true;
    }
    return false;
}
} // namespace

bool PasswordPolicy::isValid(std::string_view pw, std::string* reason) const
{
    auto set = [reason](const char* w) { if (reason) *reason = w; return false; };
    if (pw.size() < min_length)                       return set("too short");
    if (require_upper  && !hasChar(pw, &std::isupper))return set("missing uppercase");
    if (require_lower  && !hasChar(pw, &std::islower))return set("missing lowercase");
    if (require_digit  && !hasChar(pw, &std::isdigit))return set("missing digit");
    if (require_symbol && !hasSymbol(pw))             return set("missing symbol");
    return true;
}

UserManager::UserManager()
    : UserManager(defaultHasher(), {}, {})
{}

UserManager::UserManager(std::shared_ptr<IPasswordHasher> hasher,
                         TotpChecker totp,
                         Clock clock)
    : _hasher(std::move(hasher) ? std::move(hasher) : defaultHasher())
    , _totp(std::move(totp))
    , _clock(std::move(clock))
{
    if (!_totp) {
        _totp = [](std::string_view secret, std::string_view code, std::uint64_t now_s) {
            return crypto::Totp::verify(secret, now_s, code);
        };
    }
}

std::string UserManager::_generateSalt()
{
    // Deterministic but unique per call — adequate for test seeding and for
    // environments without a system RNG.  Production builds should plumb in
    // a real RNG via an optional seeder (future work).
    _salt_seed ^= _salt_seed >> 32;
    _salt_seed *= 0xBF58476D1CE4E5B9ULL;
    _salt_seed ^= _salt_seed >> 27;
    _salt_seed *= 0x94D049BB133111EBULL;
    _salt_seed ^= _salt_seed >> 31;

    std::string out(16, '\0');
    std::uint64_t s = _salt_seed;
    for (int i = 0; i < 8; ++i) {
        out[static_cast<std::size_t>(i)] = static_cast<char>(s & 0xff);
        s >>= 8;
    }
    s = _salt_seed * 0xC2B2AE3D27D4EB4FULL;
    for (int i = 0; i < 8; ++i) {
        out[static_cast<std::size_t>(8 + i)] = static_cast<char>(s & 0xff);
        s >>= 8;
    }
    return out;
}

bool UserManager::upsertUser(UserRecord r, bool overwrite)
{
    auto it = _users.find(r.user_id);
    if (it != _users.end() && !overwrite) return false;
    _users[r.user_id] = std::move(r);
    return true;
}

bool UserManager::setPassword(const std::string& user_id, std::string_view password,
                              std::string_view salt)
{
    auto it = _users.find(user_id);
    if (it == _users.end()) return false;
    if (!_policy.isValid(password)) return false;

    if (salt.empty()) {
        it->second.salt = _generateSalt();
    } else {
        it->second.salt.assign(salt);
    }
    it->second.password_encoded = _hasher->hash(password, it->second.salt);
    return true;
}

std::optional<UserRecord> UserManager::find(const std::string& user_id) const
{
    auto it = _users.find(user_id);
    if (it == _users.end()) return std::nullopt;
    return it->second;
}

bool UserManager::_isCurrentlyLocked(UserRecord& r, std::uint64_t now_ms) const
{
    if (!r.locked) return false;
    if (_lockout.duration_ms == 0) return true;  // permanent lock until admin
    if (r.lockout_started_ms == 0) return true;
    if (now_ms - r.lockout_started_ms < _lockout.duration_ms) return true;
    // Expired — auto-unlock.
    r.locked = false;
    r.failed_attempts = 0;
    r.lockout_started_ms = 0;
    return false;
}

LoginOutcome UserManager::attemptPassword(const std::string& user_id, std::string_view password)
{
    auto it = _users.find(user_id);
    if (it == _users.end()) {
        return { LoginResult::UnknownUser, "no such user" };
    }
    const std::uint64_t now = nowMs();
    if (_isCurrentlyLocked(it->second, now)) {
        return { LoginResult::AccountLocked, "account locked" };
    }

    if (!_hasher->verify(password, it->second.password_encoded)) {
        ++it->second.failed_attempts;
        if (it->second.failed_attempts >= _lockout.threshold) {
            it->second.locked = true;
            it->second.lockout_started_ms = now;
            return { LoginResult::AccountLocked, "locked after failed attempts" };
        }
        return { LoginResult::BadPassword, "wrong password" };
    }

    it->second.failed_attempts    = 0;
    it->second.lockout_started_ms = 0;
    if (!it->second.totp_secret.empty()) {
        return { LoginResult::RequiresTotp, "password ok; totp required" };
    }
    return { LoginResult::Success, "" };
}

bool UserManager::verifyTotp(std::string_view user_id, std::string_view code) const
{
    auto it = _users.find(std::string(user_id));
    if (it == _users.end()) return false;
    if (it->second.totp_secret.empty()) return false;
    if (!_totp) return false;
    const std::uint64_t now_s = nowMs() / 1000;
    return _totp(it->second.totp_secret, code, now_s);
}

void UserManager::setLocked(const std::string& user_id, bool locked)
{
    auto it = _users.find(user_id);
    if (it == _users.end()) return;
    it->second.locked = locked;
    if (!locked) {
        it->second.failed_attempts    = 0;
        it->second.lockout_started_ms = 0;
    } else {
        it->second.lockout_started_ms = nowMs();
    }
}

} // namespace m130::access

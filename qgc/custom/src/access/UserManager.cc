#include "UserManager.h"

#include <iomanip>
#include <sstream>

namespace m130::access {

std::string UserManager::defaultHash(std::string_view pw, std::string_view salt)
{
    uint64_t h = 0xcbf29ce484222325ULL;
    auto fold = [&](std::string_view s) {
        for (unsigned char c : s) { h ^= c; h *= 0x100000001b3ULL; }
    };
    fold(salt);
    fold(pw);
    fold(salt);
    std::ostringstream os;
    os << std::hex << std::setw(16) << std::setfill('0') << h;
    return os.str();
}

UserManager::UserManager() : UserManager(&defaultHash, nullptr) {}

UserManager::UserManager(Hasher h, TotpChecker t)
    : _hash(std::move(h) ? std::move(h) : &defaultHash)
    , _totp(std::move(t))
{}

bool UserManager::upsertUser(UserRecord r, bool overwrite)
{
    auto it = _users.find(r.user_id);
    if (it != _users.end() && !overwrite) {
        return false;
    }
    _users[r.user_id] = std::move(r);
    return true;
}

bool UserManager::setPassword(const std::string& user_id, std::string_view password)
{
    auto it = _users.find(user_id);
    if (it == _users.end()) return false;
    it->second.password_hash = _hash(password, it->second.salt);
    return true;
}

std::optional<UserRecord> UserManager::find(const std::string& user_id) const
{
    auto it = _users.find(user_id);
    if (it == _users.end()) return std::nullopt;
    return it->second;
}

LoginOutcome UserManager::attemptPassword(const std::string& user_id, std::string_view password)
{
    auto it = _users.find(user_id);
    if (it == _users.end()) {
        return { LoginResult::UnknownUser, "no such user" };
    }
    if (it->second.locked) {
        return { LoginResult::AccountLocked, "account locked" };
    }

    const std::string h = _hash(password, it->second.salt);
    if (h != it->second.password_hash) {
        ++it->second.failed_attempts;
        if (it->second.failed_attempts >= _lockout_threshold) {
            it->second.locked = true;
            return { LoginResult::AccountLocked, "locked after failed attempts" };
        }
        return { LoginResult::BadPassword, "wrong password" };
    }

    it->second.failed_attempts = 0;
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
    return _totp(it->second.totp_secret, code);
}

void UserManager::setLocked(const std::string& user_id, bool locked)
{
    auto it = _users.find(user_id);
    if (it == _users.end()) return;
    it->second.locked = locked;
    if (!locked) it->second.failed_attempts = 0;
}

} // namespace m130::access

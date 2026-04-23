#include "SessionManager.h"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>

namespace m130::access {

namespace {
uint64_t defaultClockMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

std::string defaultIdGen()
{
    // Non-cryptographic, but unique-enough for non-persistent sessions. The
    // hardened replacement uses libsodium randombytes_buf — follow-up PR.
    static std::atomic<uint64_t> counter{ 0 };
    thread_local std::mt19937_64 rng{ std::random_device{}() };
    const uint64_t c = counter.fetch_add(1, std::memory_order_relaxed);
    const uint64_t r = rng();
    std::ostringstream os;
    os << std::hex << std::setw(16) << std::setfill('0') << c
       << std::setw(16) << r;
    return os.str();
}
} // namespace

SessionManager::SessionManager() : SessionManager(&defaultClockMs, &defaultIdGen) {}

SessionManager::SessionManager(Clock clock, IdGenerator id_gen)
    : _clock(std::move(clock) ? std::move(clock) : &defaultClockMs)
    , _id_gen(std::move(id_gen) ? std::move(id_gen) : &defaultIdGen)
{}

bool SessionManager::isExpiredLocked(const Session& s, uint64_t now) const
{
    if (now >= s.expires_at_ms) return true;
    if (_idle_ttl_ms != 0 && now > s.last_active_ms &&
        (now - s.last_active_ms) >= _idle_ttl_ms) {
        return true;
    }
    return false;
}

std::string SessionManager::create(std::string user_id, Role role, std::optional<uint64_t> ttl_ms)
{
    const uint64_t now = _clock();
    const uint64_t ttl = ttl_ms.value_or(_default_ttl_ms);
    Session s;
    s.session_id = _id_gen();
    s.user_id    = std::move(user_id);
    s.role       = role;
    s.issued_at_ms = now;
    s.last_active_ms = now;
    s.expires_at_ms = now + ttl;

    _sessions[s.session_id] = s;
    return s.session_id;
}

std::optional<Session> SessionManager::touch(const std::string& session_id)
{
    if (_revoked.count(session_id)) return std::nullopt;
    auto it = _sessions.find(session_id);
    if (it == _sessions.end()) return std::nullopt;
    const uint64_t now = _clock();
    if (isExpiredLocked(it->second, now)) {
        _sessions.erase(it);
        return std::nullopt;
    }
    it->second.last_active_ms = now;
    return it->second;
}

SessionState SessionManager::state(const std::string& session_id) const
{
    if (_revoked.count(session_id)) return SessionState::Revoked;
    auto it = _sessions.find(session_id);
    if (it == _sessions.end()) return SessionState::NotFound;
    const uint64_t now = _clock();
    if (now >= it->second.expires_at_ms) return SessionState::Expired;
    if (_idle_ttl_ms != 0 && now > it->second.last_active_ms &&
        (now - it->second.last_active_ms) >= _idle_ttl_ms) {
        return SessionState::Idle;
    }
    return SessionState::Valid;
}

void SessionManager::revoke(const std::string& session_id)
{
    _revoked[session_id] = true;
    _sessions.erase(session_id);
}

std::size_t SessionManager::evictExpired()
{
    const uint64_t now = _clock();
    std::size_t n = 0;
    for (auto it = _sessions.begin(); it != _sessions.end();) {
        if (isExpiredLocked(it->second, now)) {
            it = _sessions.erase(it);
            ++n;
        } else {
            ++it;
        }
    }
    return n;
}

StepUpResult SessionManager::stepUp(const std::string& session_id,
                                    std::function<bool()> verify_fn,
                                    std::string* out_new_id)
{
    if (_revoked.count(session_id)) return StepUpResult::SessionInvalid;
    auto it = _sessions.find(session_id);
    if (it == _sessions.end()) return StepUpResult::SessionInvalid;
    const uint64_t now = _clock();
    if (isExpiredLocked(it->second, now)) return StepUpResult::SessionInvalid;
    if (!verify_fn || !verify_fn())       return StepUpResult::BadCode;

    Session rotated = it->second;
    _sessions.erase(it);
    rotated.session_id     = _id_gen();
    rotated.step_up_at_ms  = now;
    rotated.last_active_ms = now;
    const std::string new_id = rotated.session_id;
    _sessions[new_id] = std::move(rotated);
    if (out_new_id) *out_new_id = new_id;
    return StepUpResult::Ok;
}

bool SessionManager::requiresStepUp(const std::string& session_id, uint64_t max_age_ms) const
{
    auto it = _sessions.find(session_id);
    if (it == _sessions.end()) return true;
    if (it->second.step_up_at_ms == 0) return true;
    const uint64_t now = _clock();
    return (now - it->second.step_up_at_ms) >= max_age_ms;
}

std::size_t SessionManager::countActiveForUser(const std::string& user_id) const
{
    const uint64_t now = _clock();
    std::size_t n = 0;
    for (const auto& [id, s] : _sessions) {
        if (s.user_id == user_id && !isExpiredLocked(s, now)) {
            ++n;
        }
    }
    return n;
}

} // namespace m130::access

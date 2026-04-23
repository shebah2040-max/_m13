#include "AlertManager.h"

#include <algorithm>
#include <chrono>

namespace m130::safety {

namespace {
uint64_t defaultClockMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}
} // namespace

AlertManager::AlertManager() : AlertManager(&defaultClockMs, 256) {}

AlertManager::AlertManager(Clock clock, std::size_t max_active)
    : _clock(std::move(clock)), _max_active(max_active)
{}

bool AlertManager::raise(AlertId id, AlertLevel level, std::string title, std::string detail)
{
    ++_raised_count;

    auto it = _active.find(id);
    if (it != _active.end()) {
        // Update in place; do not downgrade.
        if (!moreSevereThan(level, it->second.level)) {
            return false;
        }
        it->second.level  = level;
        it->second.title  = std::move(title);
        it->second.detail = std::move(detail);
        it->second.raised_ms = _clock();
        publish(it->second, false);
        return true;
    }

    // Enforce bound by dropping lowest severity first (HAZ-007 mitigation).
    if (_active.size() >= _max_active) {
        auto drop_it = _active.end();
        for (auto scan = _active.begin(); scan != _active.end(); ++scan) {
            if (drop_it == _active.end()
                || moreSevereThan(drop_it->second.level, scan->second.level)) {
                drop_it = scan;
            }
        }
        if (drop_it != _active.end()) {
            _active.erase(drop_it);
        }
    }

    Alert a;
    a.raised_ms = _clock();
    a.id        = id;
    a.level     = level;
    a.title     = std::move(title);
    a.detail    = std::move(detail);
    _active.emplace(id, a);
    _history.push_back(a);
    if (_history.size() > _max_active * 8) {
        _history.pop_front();
    }
    publish(a, false);
    return true;
}

bool AlertManager::acknowledge(const AlertId& id, std::string user)
{
    auto it = _active.find(id);
    if (it == _active.end()) {
        return false;
    }
    it->second.acked_ms = _clock();
    it->second.ack_user = std::move(user);
    Alert snapshot = it->second;
    _history.push_back(snapshot);
    publish(snapshot, true);
    _active.erase(it);
    return true;
}

void AlertManager::clear(const AlertId& id)
{
    _active.erase(id);
}

AlertLevel AlertManager::masterLevel() const noexcept
{
    AlertLevel highest = AlertLevel::None;
    for (const auto& [id, a] : _active) {
        if (moreSevereThan(a.level, highest)) {
            highest = a.level;
        }
    }
    return highest;
}

std::vector<Alert> AlertManager::active() const
{
    std::vector<Alert> v;
    v.reserve(_active.size());
    for (const auto& [id, a] : _active) {
        v.push_back(a);
    }
    std::sort(v.begin(), v.end(), [](const Alert& x, const Alert& y) {
        // descending by severity; ties broken by raised_ms ascending
        if (x.level != y.level) return moreSevereThan(x.level, y.level);
        return x.raised_ms < y.raised_ms;
    });
    return v;
}

void AlertManager::subscribe(Sink s)
{
    _sinks.push_back(std::move(s));
}

void AlertManager::publish(const Alert& a, bool is_ack)
{
    for (const auto& s : _sinks) {
        if (s) s(a, is_ack);
    }
}

} // namespace m130::safety

#include "CommandAuthorization.h"

#include <chrono>
#include <deque>

namespace m130::safety {

namespace {
uint64_t defaultClockMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}
} // namespace

CommandAuthorization::CommandAuthorization() : CommandAuthorization(&defaultClockMs) {}

CommandAuthorization::CommandAuthorization(Clock clock) : _clock(std::move(clock))
{
    installDefaultPolicy();
}

void CommandAuthorization::setPolicy(std::vector<CommandPolicy> policies)
{
    _policies = std::move(policies);
    _rate.clear();
}

void CommandAuthorization::installDefaultPolicy()
{
    using access::Role;

    std::vector<CommandPolicy> p;

    CommandPolicy arm;
    arm.command = "M130_COMMAND_ARM";
    arm.min_role = Role::Operator;
    arm.allowed_phases = { FlightPhase::Prelaunch };
    arm.max_per_minute = 6;
    p.push_back(arm);

    CommandPolicy disarm;
    disarm.command = "M130_COMMAND_DISARM";
    disarm.min_role = Role::Operator;
    disarm.allowed_phases = { FlightPhase::Armed, FlightPhase::Prelaunch };
    p.push_back(disarm);

    CommandPolicy hold;
    hold.command = "M130_COMMAND_HOLD";
    hold.min_role = Role::SafetyOfficer;
    p.push_back(hold);

    CommandPolicy abort;
    abort.command = "M130_COMMAND_ABORT";
    abort.min_role = Role::SafetyOfficer;
    abort.allowed_phases = { FlightPhase::Armed, FlightPhase::Boost, FlightPhase::Cruise, FlightPhase::Terminal };
    p.push_back(abort);

    CommandPolicy fts;
    fts.command = "M130_COMMAND_FTS";
    fts.min_role = Role::RangeSafety;
    fts.dual_auth = true;
    fts.second_role_min = Role::SafetyOfficer;
    fts.allowed_phases = { FlightPhase::Armed, FlightPhase::Boost, FlightPhase::Cruise, FlightPhase::Terminal };
    p.push_back(fts);

    CommandPolicy tune;
    tune.command = "M130_COMMAND_TUNE";
    tune.min_role = Role::FlightDirector;
    tune.allowed_phases = { FlightPhase::Prelaunch, FlightPhase::Armed, FlightPhase::Cruise };
    tune.arg_ranges["param_id"] = { 0, 65535 };
    tune.max_per_minute = 60;
    p.push_back(tune);

    CommandPolicy mode;
    mode.command = "M130_COMMAND_MODE_SWITCH";
    mode.min_role = Role::Operator;
    p.push_back(mode);

    CommandPolicy checklist;
    checklist.command = "M130_COMMAND_CHECKLIST_SIGN";
    checklist.min_role = Role::Operator;
    checklist.allowed_phases = { FlightPhase::Prelaunch };
    checklist.arg_ranges["result"] = { 0, 2 };
    p.push_back(checklist);

    setPolicy(std::move(p));
}

AuthDecision CommandAuthorization::evaluate(const AuthRequest& r)
{
    const CommandPolicy* p = find(r.command);
    if (!p) {
        return { AuthResult::DeniedUnknown, "command not in policy" };
    }

    if (!access::satisfies(r.granted_role, p->min_role)) {
        return { AuthResult::DeniedRole,
                 std::string("role below ") + std::string(access::toString(p->min_role)) };
    }

    if (p->dual_auth) {
        if (!r.second_role.has_value()) {
            return { AuthResult::DeniedDualAuth, "second principal missing" };
        }
        if (!access::satisfies(*r.second_role, p->second_role_min)) {
            return { AuthResult::DeniedDualAuth, "second role insufficient" };
        }
        if (!r.second_user.has_value() || r.second_user->empty()
            || r.second_user == r.primary_user) {
            return { AuthResult::DeniedDualAuth, "same user cannot provide dual auth" };
        }
    }

    if (!p->allowed_phases.empty()) {
        bool ok = false;
        for (auto ph : p->allowed_phases) {
            if (ph == r.phase) { ok = true; break; }
        }
        if (!ok) {
            return { AuthResult::DeniedPhase,
                     std::string("phase ") + std::string(toString(r.phase)) + " not allowed" };
        }
    }

    for (const auto& [name, range] : p->arg_ranges) {
        auto it = r.args.find(name);
        if (it == r.args.end()) {
            return { AuthResult::DeniedRange, std::string("arg missing: ") + name };
        }
        if (it->second < range.first || it->second > range.second) {
            return { AuthResult::DeniedRange, std::string("arg out of range: ") + name };
        }
    }

    if (p->max_per_minute > 0) {
        const uint64_t now = r.request_time_ms != 0 ? r.request_time_ms : _clock();
        if (!recordAndCheckRate(*p, now)) {
            return { AuthResult::DeniedRate, "rate limit exceeded" };
        }
    }

    return { AuthResult::Allowed, "" };
}

const CommandPolicy* CommandAuthorization::find(std::string_view cmd) const
{
    for (const auto& p : _policies) {
        if (p.command == cmd) return &p;
    }
    return nullptr;
}

bool CommandAuthorization::recordAndCheckRate(const CommandPolicy& p, uint64_t now_ms)
{
    auto& e = _rate[p.command];
    const uint64_t window = 60000;
    while (!e.timestamps.empty() && e.timestamps.front() + window < now_ms) {
        e.timestamps.pop_front();
    }
    if (e.timestamps.size() >= p.max_per_minute) {
        return false;
    }
    e.timestamps.push_back(now_ms);
    return true;
}

std::unordered_map<std::string, uint32_t> CommandAuthorization::rateSnapshot() const
{
    std::unordered_map<std::string, uint32_t> out;
    for (const auto& [cmd, e] : _rate) {
        out[cmd] = static_cast<uint32_t>(e.timestamps.size());
    }
    return out;
}

} // namespace m130::safety

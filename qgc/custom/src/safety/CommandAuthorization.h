#pragma once

#include "../access/Role.h"
#include "FlightPhase.h"

#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace m130::safety {

/// Reason a command request may be rejected.
enum class AuthResult : uint8_t {
    Allowed           = 0,
    DeniedRole        = 1, ///< role below minimum required
    DeniedPhase       = 2, ///< command not permitted in current phase
    DeniedRange       = 3, ///< argument out of range
    DeniedRate        = 4, ///< too many of this command in the rate window
    DeniedDualAuth    = 5, ///< dual-auth required but not satisfied
    DeniedUnknown     = 6, ///< command id not in the policy
};

struct AuthRequest {
    std::string command;           ///< e.g. "M130_COMMAND_ARM"
    access::Role granted_role = access::Role::None;
    FlightPhase   phase        = FlightPhase::Unknown;
    std::unordered_map<std::string, double> args;
    uint64_t      request_time_ms = 0;
    /// Second principal (role + user). Only used for dual-auth commands.
    std::optional<access::Role> second_role;
    std::optional<std::string>  second_user;
    std::string   primary_user;
};

struct AuthDecision {
    AuthResult result = AuthResult::Allowed;
    std::string detail;
};

/// Per-command policy entry.
struct CommandPolicy {
    std::string command;
    access::Role min_role = access::Role::None;
    bool         dual_auth = false;              ///< requires second_role/user
    access::Role second_role_min = access::Role::None;
    std::vector<FlightPhase> allowed_phases;     ///< empty = any phase
    std::unordered_map<std::string, std::pair<double, double>> arg_ranges; ///< name → [lo,hi]
    uint32_t     max_per_minute = 0;              ///< 0 = unlimited
};

/// Central RBAC + sanity check authority (REQ-M130-GCS-CMD-001/004).
///
/// Default-deny: any command not in the policy is rejected with DeniedUnknown.
/// The Foundation policy ships a conservative baseline; operators may swap
/// policies via `setPolicy` but only from Admin-initiated flows.
class CommandAuthorization
{
public:
    using Clock = std::function<uint64_t()>;

    CommandAuthorization();
    explicit CommandAuthorization(Clock clock);

    /// Replace the entire policy. Useful for Admin reconfiguration or tests.
    void setPolicy(std::vector<CommandPolicy> policies);

    /// Install the Foundation baseline policy (matches ICD-MAVLink.md).
    void installDefaultPolicy();

    AuthDecision evaluate(const AuthRequest& r);

    /// Observability — per-command rate counters in the current window.
    std::unordered_map<std::string, uint32_t> rateSnapshot() const;

private:
    Clock _clock;
    std::vector<CommandPolicy> _policies;

    struct RateEntry {
        std::deque<uint64_t> timestamps; ///< ms
    };
    mutable std::unordered_map<std::string, RateEntry> _rate;

    const CommandPolicy* find(std::string_view cmd) const;
    bool recordAndCheckRate(const CommandPolicy& p, uint64_t now_ms);
};

} // namespace m130::safety

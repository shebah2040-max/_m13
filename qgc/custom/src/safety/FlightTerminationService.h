#pragma once

#include "../access/Role.h"
#include "AlertLevel.h"
#include "FlightPhase.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace m130::safety {

/// Why a FTS was (or is being) attempted.
enum class FtsReason : uint8_t {
    ManualRso           = 0,
    EnvelopeViolation   = 1,
    LossOfControl       = 2,
    LinkLoss            = 3,
    IipOutOfCorridor    = 4,
    BatteryCritical     = 5,
};

/// Dual-auth FTS request (REQ-M130-GCS-SAFE-009, REQ-M130-GCS-SAFE-010).
struct FtsRequest {
    std::string primary_user;          ///< RSO identifier
    access::Role primary_role = access::Role::None;
    std::string primary_totp;          ///< 6-8 digit code (verified externally)

    std::string secondary_user;        ///< Safety Officer identifier
    access::Role secondary_role = access::Role::None;
    std::string secondary_totp;

    uint32_t flight_id   = 0;
    uint64_t timestamp_ms = 0;          ///< request creation time
    FtsReason reason      = FtsReason::ManualRso;
    uint8_t   command_type = 0;         ///< 0=full FTS, 1=engine cutoff
};

enum class FtsResult : uint8_t {
    ArmedAndSent       = 0, ///< Fully authorized and dispatched
    RejectedPrimaryAuth   = 1,
    RejectedSecondaryAuth = 2,
    RejectedSameUser      = 3,
    RejectedRoles         = 4,
    RejectedExpired       = 5,
    RejectedNotArmed      = 6,
};

struct FtsDecision {
    FtsResult result = FtsResult::ArmedAndSent;
    std::string detail;
    std::uint64_t decided_at_ms = 0;
};

/// Verifies a TOTP code for a given user. Supplied by the Access layer.
using TotpVerifier = std::function<bool(std::string_view user, std::string_view code)>;

/// Sink that actually dispatches the FTS on the wire.
using FtsDispatcher = std::function<void(const FtsRequest&)>;

/// Orchestrator for FTS — enforces policy before dispatch.
class FlightTerminationService
{
public:
    using Clock = std::function<uint64_t()>;

    FlightTerminationService();
    FlightTerminationService(TotpVerifier verifier, FtsDispatcher dispatch, Clock clock);

    /// Allowable age of an FtsRequest before it is considered expired (ms).
    void setRequestTtlMs(uint64_t ttl) { _ttl_ms = ttl; }

    /// Current mission phase — used to reject FTS when on ground.
    void setPhase(FlightPhase phase) { _phase = phase; }

    FtsDecision armAndFire(const FtsRequest& req);

    std::uint64_t totalAttempts() const noexcept { return _attempts; }
    std::uint64_t totalArmed()    const noexcept { return _armed; }

private:
    TotpVerifier  _verifier;
    FtsDispatcher _dispatch;
    Clock         _clock;
    uint64_t      _ttl_ms = 30000; ///< 30 s per ICD §3.4
    FlightPhase   _phase  = FlightPhase::Unknown;
    std::uint64_t _attempts = 0;
    std::uint64_t _armed    = 0;
};

} // namespace m130::safety

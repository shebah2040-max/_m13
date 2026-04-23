#include "FlightTerminationService.h"

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

FlightTerminationService::FlightTerminationService()
    : FlightTerminationService(nullptr, nullptr, &defaultClockMs)
{}

FlightTerminationService::FlightTerminationService(TotpVerifier verifier, FtsDispatcher dispatch, Clock clock)
    : _verifier(std::move(verifier))
    , _dispatch(std::move(dispatch))
    , _clock(std::move(clock) ? std::move(clock) : &defaultClockMs)
{}

FtsDecision FlightTerminationService::armAndFire(const FtsRequest& req)
{
    ++_attempts;
    FtsDecision d;
    d.decided_at_ms = _clock();

    if (!access::satisfies(req.primary_role, access::Role::RangeSafety)) {
        d.result = FtsResult::RejectedRoles;
        d.detail = "primary must be RangeSafety or higher";
        return d;
    }
    if (!access::satisfies(req.secondary_role, access::Role::SafetyOfficer)) {
        d.result = FtsResult::RejectedRoles;
        d.detail = "secondary must be SafetyOfficer or higher";
        return d;
    }
    if (req.primary_user.empty() || req.secondary_user.empty()
        || req.primary_user == req.secondary_user) {
        d.result = FtsResult::RejectedSameUser;
        d.detail = "dual-auth requires two distinct users";
        return d;
    }

    if (_phase != FlightPhase::Armed
        && !isInFlight(_phase)) {
        d.result = FtsResult::RejectedNotArmed;
        d.detail = "FTS only valid when armed or in flight";
        return d;
    }

    if (req.timestamp_ms == 0 || (d.decided_at_ms - req.timestamp_ms) > _ttl_ms) {
        d.result = FtsResult::RejectedExpired;
        d.detail = "request expired or missing timestamp";
        return d;
    }

    if (!_verifier || !_verifier(req.primary_user, req.primary_totp)) {
        d.result = FtsResult::RejectedPrimaryAuth;
        d.detail = "primary TOTP invalid";
        return d;
    }
    if (!_verifier(req.secondary_user, req.secondary_totp)) {
        d.result = FtsResult::RejectedSecondaryAuth;
        d.detail = "secondary TOTP invalid";
        return d;
    }

    // All checks passed — dispatch.
    if (_dispatch) {
        _dispatch(req);
    }
    ++_armed;
    d.result = FtsResult::ArmedAndSent;
    d.detail = "FTS armed and dispatched";
    return d;
}

} // namespace m130::safety

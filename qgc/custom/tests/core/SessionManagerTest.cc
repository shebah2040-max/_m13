#include "access/SessionManager.h"
#include "test_support.h"

#include <atomic>
#include <memory>
#include <string>

using namespace m130::access;

namespace {

struct FakeClock {
    std::uint64_t now_ms = 100000;
    std::uint64_t operator()() { return now_ms; }
};

} // namespace

int createAndValidate()
{
    auto clock = std::make_shared<FakeClock>();
    std::atomic<int> n{0};
    SessionManager sm([clock]() { return clock->now_ms; },
                      [&n]() { return std::string("sess-") + std::to_string(++n); });
    sm.setIdleTtlMs(0);  // disable idle for this test

    const std::string sid = sm.create("alice", Role::Operator);
    M130_REQUIRE_EQ(sm.state(sid), SessionState::Valid);
    auto s = sm.touch(sid);
    M130_REQUIRE(s.has_value());
    M130_REQUIRE_EQ(s->user_id, std::string("alice"));
    return 0;
}

int idleTimeoutEvicts()
{
    auto clock = std::make_shared<FakeClock>();
    SessionManager sm([clock]() { return clock->now_ms; }, nullptr);
    sm.setDefaultTtlMs(60 * 60 * 1000); // 1h absolute
    sm.setIdleTtlMs(10 * 1000);         // 10s idle

    const std::string sid = sm.create("alice", Role::Operator);
    clock->now_ms += 5 * 1000;
    M130_REQUIRE_EQ(sm.state(sid), SessionState::Valid);
    clock->now_ms += 10 * 1000;
    M130_REQUIRE_EQ(sm.state(sid), SessionState::Idle);
    M130_REQUIRE_EQ(sm.evictExpired(), 1u);
    M130_REQUIRE_EQ(sm.state(sid), SessionState::NotFound);
    return 0;
}

int absoluteTtlExpires()
{
    auto clock = std::make_shared<FakeClock>();
    SessionManager sm([clock]() { return clock->now_ms; }, nullptr);
    sm.setIdleTtlMs(0);

    const std::string sid = sm.create("alice", Role::Operator, 500);
    clock->now_ms += 600;
    M130_REQUIRE_EQ(sm.state(sid), SessionState::Expired);
    return 0;
}

int stepUpRotatesSessionId()
{
    auto clock = std::make_shared<FakeClock>();
    std::atomic<int> n{0};
    SessionManager sm([clock]() { return clock->now_ms; },
                      [&n]() { return std::string("sess-") + std::to_string(++n); });
    sm.setIdleTtlMs(0);

    const std::string sid = sm.create("alice", Role::RangeSafety);
    M130_REQUIRE(sm.requiresStepUp(sid, 60 * 1000));

    std::string new_id;
    M130_REQUIRE_EQ(sm.stepUp(sid, []() { return true; }, &new_id), StepUpResult::Ok);
    M130_REQUIRE(!new_id.empty());
    M130_REQUIRE(new_id != sid);
    M130_REQUIRE_EQ(sm.state(sid),    SessionState::NotFound);
    M130_REQUIRE_EQ(sm.state(new_id), SessionState::Valid);
    M130_REQUIRE(!sm.requiresStepUp(new_id, 60 * 1000));

    clock->now_ms += 61 * 1000;
    M130_REQUIRE(sm.requiresStepUp(new_id, 60 * 1000));
    return 0;
}

int stepUpBadCodeReturnsBadCode()
{
    SessionManager sm;
    sm.setIdleTtlMs(0);
    const std::string sid = sm.create("alice", Role::RangeSafety);
    M130_REQUIRE_EQ(sm.stepUp(sid, []() { return false; }), StepUpResult::BadCode);
    M130_REQUIRE_EQ(sm.state(sid), SessionState::Valid);  // unchanged
    return 0;
}

int stepUpOnInvalidSession()
{
    SessionManager sm;
    M130_REQUIRE_EQ(sm.stepUp("nonsense", []() { return true; }), StepUpResult::SessionInvalid);
    return 0;
}

int run()
{
    M130_RUN(createAndValidate);
    M130_RUN(idleTimeoutEvicts);
    M130_RUN(absoluteTtlExpires);
    M130_RUN(stepUpRotatesSessionId);
    M130_RUN(stepUpBadCodeReturnsBadCode);
    M130_RUN(stepUpOnInvalidSession);
    return 0;
}

M130_TEST_MAIN()

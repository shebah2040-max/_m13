#include "access/UserManager.h"
#include "access/PasswordHasher.h"
#include "test_support.h"

#include <memory>
#include <string>
#include <string_view>

using namespace m130::access;

namespace {

struct FakeClock {
    std::uint64_t now_ms = 100000;
    std::uint64_t operator()() { return now_ms; }
};

UserRecord makeUser(std::string id, Role role)
{
    UserRecord r;
    r.user_id = std::move(id);
    r.display_name = "display";
    r.role = role;
    return r;
}

} // namespace

int createLoginAndLockout()
{
    // Fast hasher (low iterations) to keep the test cheap.
    auto hasher = std::make_shared<Pbkdf2Hasher>(1024);
    auto clock_state = std::make_shared<FakeClock>();
    UserManager um(hasher, {}, [clock_state]() { return clock_state->now_ms; });

    LockoutPolicy lp;
    lp.threshold = 3;
    lp.duration_ms = 60 * 1000;
    um.setLockoutPolicy(lp);

    M130_REQUIRE(um.upsertUser(makeUser("alice", Role::Operator)));
    M130_REQUIRE(um.setPassword("alice", "Correct-Horse-9!"));

    M130_REQUIRE_EQ(um.attemptPassword("alice", "Correct-Horse-9!").result, LoginResult::Success);
    M130_REQUIRE_EQ(um.attemptPassword("alice", "wrong").result, LoginResult::BadPassword);
    M130_REQUIRE_EQ(um.attemptPassword("alice", "wrong").result, LoginResult::BadPassword);
    M130_REQUIRE_EQ(um.attemptPassword("alice", "wrong").result, LoginResult::AccountLocked);
    M130_REQUIRE_EQ(um.attemptPassword("alice", "Correct-Horse-9!").result, LoginResult::AccountLocked);
    return 0;
}

int lockoutAutoUnlocksAfterDuration()
{
    auto hasher = std::make_shared<Pbkdf2Hasher>(1024);
    auto clock_state = std::make_shared<FakeClock>();
    UserManager um(hasher, {}, [clock_state]() { return clock_state->now_ms; });
    um.setLockoutPolicy({ 2, 30 * 1000 });

    M130_REQUIRE(um.upsertUser(makeUser("bob", Role::Operator)));
    M130_REQUIRE(um.setPassword("bob", "Correct-Horse-9!"));

    M130_REQUIRE_EQ(um.attemptPassword("bob", "wrong").result, LoginResult::BadPassword);
    M130_REQUIRE_EQ(um.attemptPassword("bob", "wrong").result, LoginResult::AccountLocked);
    clock_state->now_ms += 30 * 1000;
    // Duration elapsed — next attempt auto-unlocks and evaluates the password.
    M130_REQUIRE_EQ(um.attemptPassword("bob", "Correct-Horse-9!").result, LoginResult::Success);
    return 0;
}

int unknownUser()
{
    UserManager um;
    M130_REQUIRE_EQ(um.attemptPassword("nobody", "x").result, LoginResult::UnknownUser);
    return 0;
}

int passwordPolicyRejectsWeakPasswords()
{
    auto hasher = std::make_shared<Pbkdf2Hasher>(1024);
    UserManager um(hasher);
    M130_REQUIRE(um.upsertUser(makeUser("carol", Role::Observer)));
    // All below violate the default policy.
    M130_REQUIRE(!um.setPassword("carol", "short"));          // too short
    M130_REQUIRE(!um.setPassword("carol", "alllowercase123")); // no upper / symbol
    M130_REQUIRE(!um.setPassword("carol", "ALLUPPERCASE123")); // no lower / symbol
    M130_REQUIRE(!um.setPassword("carol", "NoDigitsOrSym!"));  // no digit
    M130_REQUIRE(!um.setPassword("carol", "NoSymbol12345"));   // no symbol
    M130_REQUIRE( um.setPassword("carol", "Strong-Pass-9!"));  // all requirements met
    return 0;
}

int requiresTotpWhenSecretSet()
{
    auto hasher = std::make_shared<Pbkdf2Hasher>(1024);
    UserManager um(hasher, [](std::string_view secret, std::string_view code, std::uint64_t) {
        return secret == code;
    });

    auto r = makeUser("dave", Role::FlightDirector);
    r.totp_secret = "seed";
    M130_REQUIRE(um.upsertUser(r));
    M130_REQUIRE(um.setPassword("dave", "Strong-Pass-9!"));

    auto o = um.attemptPassword("dave", "Strong-Pass-9!");
    M130_REQUIRE_EQ(o.result, LoginResult::RequiresTotp);
    M130_REQUIRE(um.verifyTotp("dave", "seed"));
    M130_REQUIRE(!um.verifyTotp("dave", "bad"));
    return 0;
}

int run()
{
    M130_RUN(createLoginAndLockout);
    M130_RUN(lockoutAutoUnlocksAfterDuration);
    M130_RUN(unknownUser);
    M130_RUN(passwordPolicyRejectsWeakPasswords);
    M130_RUN(requiresTotpWhenSecretSet);
    return 0;
}

M130_TEST_MAIN()

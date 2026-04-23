#include "access/UserManager.h"
#include "test_support.h"

using namespace m130::access;

int createLoginAndLockout()
{
    UserManager um;
    um.setLockoutThreshold(3);

    UserRecord r;
    r.user_id = "alice";
    r.display_name = "Alice";
    r.role = Role::Operator;
    r.salt = "s";
    r.password_hash = ""; // set via setPassword
    M130_REQUIRE(um.upsertUser(r));
    M130_REQUIRE(um.setPassword("alice", "P@ss"));

    M130_REQUIRE_EQ(um.attemptPassword("alice", "P@ss").result,  LoginResult::Success);
    M130_REQUIRE_EQ(um.attemptPassword("alice", "wrong").result, LoginResult::BadPassword);
    M130_REQUIRE_EQ(um.attemptPassword("alice", "wrong").result, LoginResult::BadPassword);
    M130_REQUIRE_EQ(um.attemptPassword("alice", "wrong").result, LoginResult::AccountLocked);
    M130_REQUIRE_EQ(um.attemptPassword("alice", "P@ss").result,  LoginResult::AccountLocked);
    return 0;
}

int unknownUser()
{
    UserManager um;
    M130_REQUIRE_EQ(um.attemptPassword("nobody", "x").result, LoginResult::UnknownUser);
    return 0;
}

int requiresTotpWhenSecretSet()
{
    UserManager um(
        [](std::string_view pw, std::string_view){ return std::string(pw); },
        [](std::string_view secret, std::string_view code) { return secret == code; });

    UserRecord r;
    r.user_id = "bob";
    r.role = Role::FlightDirector;
    r.password_hash = std::string("good");
    r.totp_secret = "abc";
    M130_REQUIRE(um.upsertUser(r));
    auto o = um.attemptPassword("bob", "good");
    M130_REQUIRE_EQ(o.result, LoginResult::RequiresTotp);
    M130_REQUIRE(um.verifyTotp("bob", "abc"));
    M130_REQUIRE(!um.verifyTotp("bob", "bad"));
    return 0;
}

int run()
{
    M130_RUN(createLoginAndLockout);
    M130_RUN(unknownUser);
    M130_RUN(requiresTotpWhenSecretSet);
    return 0;
}

M130_TEST_MAIN()

#include "access/Authenticator.h"
#include "access/UserManager.h"
#include "access/PasswordHasher.h"
#include "test_support.h"

#include <memory>

using namespace m130::access;

namespace {

class AlwaysUnhandled final : public IAuthenticator
{
public:
    std::string id() const override { return "stub"; }
    AuthResult  authenticate(const AuthContext&) override { return { AuthStatus::NotHandled, Role::None, "" }; }
};

class AlwaysSucceeds final : public IAuthenticator
{
public:
    AlwaysSucceeds(Role r) : _r(r) {}
    std::string id() const override { return "succ"; }
    AuthResult  authenticate(const AuthContext&) override { return { AuthStatus::Success, _r, "ok" }; }
private:
    Role _r;
};

} // namespace

int chainReturnsFirstHandled()
{
    AuthenticatorChain chain;
    chain.add(std::make_shared<AlwaysUnhandled>());
    chain.add(std::make_shared<AlwaysSucceeds>(Role::FlightDirector));
    auto r = chain.authenticate({"u","p","",""});
    M130_REQUIRE_EQ(r.status, AuthStatus::Success);
    M130_REQUIRE_EQ(r.role,   Role::FlightDirector);
    return 0;
}

int chainReturnsNotHandledWhenEmpty()
{
    AuthenticatorChain chain;
    auto r = chain.authenticate({"u","p","",""});
    M130_REQUIRE_EQ(r.status, AuthStatus::NotHandled);
    return 0;
}

int localAuthenticatorSuccess()
{
    auto hasher = std::make_shared<Pbkdf2Hasher>(1024);
    UserManager um(hasher);
    UserRecord u; u.user_id = "alice"; u.role = Role::Operator;
    M130_REQUIRE(um.upsertUser(u));
    M130_REQUIRE(um.setPassword("alice", "Strong-Pass-9!"));

    LocalAuthenticator la(&um);
    auto r = la.authenticate({"alice", "Strong-Pass-9!", "", ""});
    M130_REQUIRE_EQ(r.status, AuthStatus::Success);
    M130_REQUIRE_EQ(r.role,   Role::Operator);
    return 0;
}

int localAuthenticatorRequiresTotpWhenConfigured()
{
    auto hasher = std::make_shared<Pbkdf2Hasher>(1024);
    UserManager um(hasher, [](std::string_view s, std::string_view c, std::uint64_t) {
        return s == c;
    });
    UserRecord u; u.user_id = "bob"; u.role = Role::RangeSafety; u.totp_secret = "shared";
    M130_REQUIRE(um.upsertUser(u));
    M130_REQUIRE(um.setPassword("bob", "Strong-Pass-9!"));

    LocalAuthenticator la(&um);
    auto r1 = la.authenticate({"bob", "Strong-Pass-9!", "", ""});
    M130_REQUIRE_EQ(r1.status, AuthStatus::RequiresTotp);
    auto r2 = la.authenticate({"bob", "Strong-Pass-9!", "shared", ""});
    M130_REQUIRE_EQ(r2.status, AuthStatus::Success);
    auto r3 = la.authenticate({"bob", "Strong-Pass-9!", "wrong", ""});
    M130_REQUIRE_EQ(r3.status, AuthStatus::BadTotp);
    return 0;
}

int run()
{
    M130_RUN(chainReturnsFirstHandled);
    M130_RUN(chainReturnsNotHandledWhenEmpty);
    M130_RUN(localAuthenticatorSuccess);
    M130_RUN(localAuthenticatorRequiresTotpWhenConfigured);
    return 0;
}

M130_TEST_MAIN()

#include "access/LdapAuthenticator.h"
#include "access/Role.h"
#include "test_support.h"

#include <memory>

using namespace m130::access;

namespace {

std::shared_ptr<InMemoryLdap> makeDirectory()
{
    auto dir = std::make_shared<InMemoryLdap>();

    InMemoryLdap::Entry alice;
    alice.password = "alicePass!";
    alice.attrs["objectclass"] = {"inetOrgPerson"};
    alice.attrs["uid"]         = {"alice"};
    alice.attrs["memberof"]    = {"cn=operators,ou=Groups,dc=m130,dc=local"};
    dir->add("uid=alice,ou=People,dc=m130,dc=local", std::move(alice));

    InMemoryLdap::Entry bob;
    bob.password = "bobPass!";
    bob.attrs["objectclass"] = {"inetOrgPerson"};
    bob.attrs["uid"]         = {"bob"};
    bob.attrs["memberof"]    = {"cn=observers,ou=Groups,dc=m130,dc=local"};
    dir->add("uid=bob,ou=People,dc=m130,dc=local", std::move(bob));

    return dir;
}

LdapConfig defaultConfig()
{
    LdapConfig cfg;
    cfg.role_rules = {
        {"cn=flight-directors,ou=Groups,dc=m130,dc=local", Role::FlightDirector},
        {"cn=operators,ou=Groups,dc=m130,dc=local",        Role::Operator},
        {"cn=observers,ou=Groups,dc=m130,dc=local",        Role::Observer},
    };
    cfg.fallback_role = Role::None;
    return cfg;
}

} // namespace

int assignsRoleFromMembership()
{
    LdapAuthenticator auth(makeDirectory(), defaultConfig());

    AuthContext ctx;
    ctx.user_id  = "alice";
    ctx.password = "alicePass!";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::Success);
    M130_REQUIRE(r.role   == Role::Operator);
    return 0;
}

int rejectsBadPassword()
{
    LdapAuthenticator auth(makeDirectory(), defaultConfig());
    AuthContext ctx;
    ctx.user_id  = "alice";
    ctx.password = "wrong";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::BadPassword);
    return 0;
}

int rejectsUnknownUser()
{
    LdapAuthenticator auth(makeDirectory(), defaultConfig());
    AuthContext ctx;
    ctx.user_id  = "ghost";
    ctx.password = "anything";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::BadPassword);
    return 0;
}

int rejectsWhenNoRoleMatches()
{
    auto dir = makeDirectory();

    InMemoryLdap::Entry stray;
    stray.password = "strayPass!";
    stray.attrs["objectclass"] = {"inetOrgPerson"};
    stray.attrs["uid"]         = {"stray"};
    stray.attrs["memberof"]    = {"cn=visitors,ou=Groups,dc=m130,dc=local"};
    dir->add("uid=stray,ou=People,dc=m130,dc=local", std::move(stray));

    LdapAuthenticator auth(dir, defaultConfig());
    AuthContext ctx;
    ctx.user_id  = "stray";
    ctx.password = "strayPass!";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::PolicyViolation);
    return 0;
}

int integratesWithAuthenticatorChain()
{
    auto chain = std::make_shared<AuthenticatorChain>();
    auto ldap  = std::make_shared<LdapAuthenticator>(makeDirectory(), defaultConfig());
    chain->add(ldap);

    AuthContext ctx;
    ctx.user_id  = "bob";
    ctx.password = "bobPass!";
    const auto r = chain->authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::Success);
    M130_REQUIRE(r.role   == Role::Observer);
    return 0;
}

int run()
{
    M130_RUN(assignsRoleFromMembership);
    M130_RUN(rejectsBadPassword);
    M130_RUN(rejectsUnknownUser);
    M130_RUN(rejectsWhenNoRoleMatches);
    M130_RUN(integratesWithAuthenticatorChain);
    return 0;
}

M130_TEST_MAIN()

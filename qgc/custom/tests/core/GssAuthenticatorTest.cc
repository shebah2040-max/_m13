#include "access/GssAuthenticator.h"
#include "access/Authenticator.h"
#include "test_support.h"

#include <memory>

using namespace m130::access;

int stubProviderAlwaysReturnsNotHandled()
{
    GssPolicy policy;
    policy.mapRealm("REALM.LOCAL", Role::Operator);
    GssAuthenticator auth(std::make_shared<StubGssProvider>(), std::move(policy));

    AuthContext ctx;
    ctx.gss_token = "any-token";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::BadPassword);
    return 0;
}

int skipsWhenNoTokenPresented()
{
    auto fake = std::make_shared<FakeGssProvider>();
    GssAuthenticator auth(fake, {});
    AuthContext ctx;
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::NotHandled);
    return 0;
}

int mapsPrincipalToRoleExactMatch()
{
    auto fake = std::make_shared<FakeGssProvider>();
    fake->accept("tok-alice", "alice@REALM.LOCAL");

    GssPolicy policy;
    policy.mapPrincipal("alice@realm.local", Role::FlightDirector);
    GssAuthenticator auth(fake, std::move(policy));

    AuthContext ctx;
    ctx.gss_token = "tok-alice";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::Success);
    M130_REQUIRE(r.role   == Role::FlightDirector);
    return 0;
}

int mapsRealmToRoleWhenPrincipalNotRegistered()
{
    auto fake = std::make_shared<FakeGssProvider>();
    fake->accept("tok-bob", "bob@RANGE.OPS");

    GssPolicy policy;
    policy.mapRealm("range.ops", Role::Observer);
    GssAuthenticator auth(fake, std::move(policy));

    AuthContext ctx;
    ctx.gss_token = "tok-bob";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::Success);
    M130_REQUIRE(r.role   == Role::Observer);
    return 0;
}

int rejectsPrincipalWithoutMapping()
{
    auto fake = std::make_shared<FakeGssProvider>();
    fake->accept("tok-ghost", "ghost@OTHER.REALM");

    GssPolicy policy; // empty
    GssAuthenticator auth(fake, std::move(policy));

    AuthContext ctx;
    ctx.gss_token = "tok-ghost";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::PolicyViolation);
    return 0;
}

int rejectsUnknownToken()
{
    auto fake = std::make_shared<FakeGssProvider>();
    GssPolicy policy;
    policy.mapRealm("realm.local", Role::Operator);
    GssAuthenticator auth(fake, std::move(policy));

    AuthContext ctx;
    ctx.gss_token = "not-provisioned";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::BadPassword);
    return 0;
}

int chainIntegration()
{
    auto chain = std::make_shared<AuthenticatorChain>();
    auto fake  = std::make_shared<FakeGssProvider>();
    fake->accept("tok-director", "director@REALM.LOCAL");

    GssPolicy policy;
    policy.mapRealm("realm.local", Role::FlightDirector);
    chain->add(std::make_shared<GssAuthenticator>(fake, std::move(policy)));

    AuthContext ctx;
    ctx.gss_token = "tok-director";
    const auto r = chain->authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::Success);
    M130_REQUIRE(r.role   == Role::FlightDirector);
    return 0;
}

int run()
{
    M130_RUN(stubProviderAlwaysReturnsNotHandled);
    M130_RUN(skipsWhenNoTokenPresented);
    M130_RUN(mapsPrincipalToRoleExactMatch);
    M130_RUN(mapsRealmToRoleWhenPrincipalNotRegistered);
    M130_RUN(rejectsPrincipalWithoutMapping);
    M130_RUN(rejectsUnknownToken);
    M130_RUN(chainIntegration);
    return 0;
}

M130_TEST_MAIN()

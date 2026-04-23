#include "access/CertPolicy.h"
#include "access/MtlsAuthenticator.h"
#include "test_support.h"

#include <chrono>
#include <memory>

using namespace m130::access;
using clock_type = std::chrono::system_clock;

namespace {

X509CertFields goodCert(clock_type::time_point now)
{
    X509CertFields c;
    c.subject_cn = "alice.ops.m130";
    c.issuer_cn  = "M130 Internal CA";
    c.subject_alt_names = {"alice.ops.m130", "alice@m130.local"};
    c.extended_key_usages = {"clientAuth"};
    c.fingerprint_sha256_hex =
        "1b2c3d4e5f60718293a4b5c6d7e8f90112233445566778899aabbccddeeff001";
    c.not_before = now - std::chrono::hours(24);
    c.not_after  = now + std::chrono::hours(24 * 30);
    c.issuer_chain = {"M130 Internal CA", "M130 Root CA"};
    return c;
}

} // namespace

int allowsValidCertWithoutAnyPolicy()
{
    const auto now = clock_type::now();
    CertPolicy p;
    const auto ev = p.evaluate(goodCert(now), now);
    M130_REQUIRE(ev.decision == CertDecision::Allow);
    return 0;
}

int pinningMarksDecisionAsPinned()
{
    const auto now = clock_type::now();
    CertPolicy p;
    p.pinFingerprint(
        "1b2c3d4e5f60718293a4b5c6d7e8f90112233445566778899aabbccddeeff001");
    const auto ev = p.evaluate(goodCert(now), now);
    M130_REQUIRE(ev.decision == CertDecision::Pinned);
    return 0;
}

int requiresPinnedRejectsUnpinnedCert()
{
    const auto now = clock_type::now();
    CertPolicy p;
    p.setRequirePinned(true);
    p.pinFingerprint("deadbeef");
    const auto ev = p.evaluate(goodCert(now), now);
    M130_REQUIRE(ev.decision == CertDecision::NotPinned);
    return 0;
}

int rejectsExpiredCert()
{
    const auto now = clock_type::now();
    auto cert = goodCert(now);
    cert.not_after = now - std::chrono::hours(1);
    CertPolicy p;
    const auto ev = p.evaluate(cert, now);
    M130_REQUIRE(ev.decision == CertDecision::Expired);
    return 0;
}

int rejectsNotYetValidCert()
{
    const auto now = clock_type::now();
    auto cert = goodCert(now);
    cert.not_before = now + std::chrono::hours(1);
    CertPolicy p;
    const auto ev = p.evaluate(cert, now);
    M130_REQUIRE(ev.decision == CertDecision::NotYetValid);
    return 0;
}

int rejectsWhenNoSanMatches()
{
    const auto now = clock_type::now();
    auto cert = goodCert(now);
    CertPolicy p;
    p.addAllowedSan("peer.range.m130");
    const auto ev = p.evaluate(cert, now);
    M130_REQUIRE(ev.decision == CertDecision::SanMismatch);

    p.addAllowedSan("alice.ops.m130");
    const auto ok = p.evaluate(cert, now);
    M130_REQUIRE(ok.decision == CertDecision::Allow);
    return 0;
}

int rejectsWhenEkuMissing()
{
    const auto now = clock_type::now();
    auto cert = goodCert(now);
    CertPolicy p;
    p.addRequiredEku("serverAuth");
    const auto ev = p.evaluate(cert, now);
    M130_REQUIRE(ev.decision == CertDecision::MissingEku);
    return 0;
}

int rejectsUntrustedIssuer()
{
    const auto now = clock_type::now();
    auto cert = goodCert(now);
    CertPolicy p;
    p.addTrustedIssuer("Another CA");
    const auto ev = p.evaluate(cert, now);
    M130_REQUIRE(ev.decision == CertDecision::UntrustedIssuer);

    p.addTrustedIssuer("M130 Root CA");
    const auto ok = p.evaluate(cert, now);
    M130_REQUIRE(ok.decision == CertDecision::Allow);
    return 0;
}

int mtlsAuthenticatorReturnsMappedRole()
{
    const auto now = clock_type::now();
    CertPolicy p;
    p.addTrustedIssuer("M130 Root CA");
    p.addRequiredEku("clientAuth");
    p.mapRole("alice.ops.m130", Role::Operator);

    auto channel = std::make_shared<PolicyEnforcer>(std::move(p));
    MtlsAuthenticator auth(channel);
    auth.setPresentedCert(goodCert(now), now);

    AuthContext ctx;
    ctx.mtls_subject_cn = "alice.ops.m130";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::Success);
    M130_REQUIRE(r.role   == Role::Operator);
    return 0;
}

int mtlsAuthenticatorRejectsPolicyFailure()
{
    const auto now = clock_type::now();
    CertPolicy p;
    p.setRequirePinned(true);
    p.pinFingerprint("deadbeef");
    p.mapRole("alice.ops.m130", Role::Operator);

    auto channel = std::make_shared<PolicyEnforcer>(std::move(p));
    MtlsAuthenticator auth(channel);
    auth.setPresentedCert(goodCert(now), now);

    AuthContext ctx;
    ctx.mtls_subject_cn = "alice.ops.m130";
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::PolicyViolation);
    return 0;
}

int mtlsAuthenticatorSkipsWhenNoCnPresented()
{
    CertPolicy p;
    auto channel = std::make_shared<PolicyEnforcer>(std::move(p));
    MtlsAuthenticator auth(channel);

    AuthContext ctx;
    const auto r = auth.authenticate(ctx);
    M130_REQUIRE(r.status == AuthStatus::NotHandled);
    return 0;
}

int run()
{
    M130_RUN(allowsValidCertWithoutAnyPolicy);
    M130_RUN(pinningMarksDecisionAsPinned);
    M130_RUN(requiresPinnedRejectsUnpinnedCert);
    M130_RUN(rejectsExpiredCert);
    M130_RUN(rejectsNotYetValidCert);
    M130_RUN(rejectsWhenNoSanMatches);
    M130_RUN(rejectsWhenEkuMissing);
    M130_RUN(rejectsUntrustedIssuer);
    M130_RUN(mtlsAuthenticatorReturnsMappedRole);
    M130_RUN(mtlsAuthenticatorRejectsPolicyFailure);
    M130_RUN(mtlsAuthenticatorSkipsWhenNoCnPresented);
    return 0;
}

M130_TEST_MAIN()

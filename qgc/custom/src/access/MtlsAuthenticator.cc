#include "MtlsAuthenticator.h"

namespace m130::access {

void MtlsAuthenticator::setPresentedCert(const X509CertFields& cert,
                                         std::chrono::system_clock::time_point now)
{
    _cert = cert;
    _cert_now = now;
    _cert_presented = true;
}

void MtlsAuthenticator::clearPresentedCert()
{
    _cert_presented = false;
    _cert = {};
    _cert_now = {};
}

AuthResult MtlsAuthenticator::authenticate(const AuthContext& ctx)
{
    AuthResult r;

    if (!_channel) {
        r.status = AuthStatus::NotHandled;
        return r;
    }
    if (ctx.mtls_subject_cn.empty()) {
        r.status = AuthStatus::NotHandled;
        return r;
    }

    if (_cert_presented) {
        const auto ev = _channel->evaluate(_cert, _cert_now);
        switch (ev.decision) {
        case CertDecision::Allow:
        case CertDecision::Pinned:
            break;
        case CertDecision::Expired:
        case CertDecision::NotYetValid:
        case CertDecision::SanMismatch:
        case CertDecision::MissingEku:
        case CertDecision::UntrustedIssuer:
        case CertDecision::NotPinned:
        case CertDecision::MissingFingerprint:
            r.status = AuthStatus::PolicyViolation;
            r.detail = ev.detail;
            return r;
        }
    }

    const Role role = _channel->lookupRole(ctx.mtls_subject_cn);
    if (role == Role::None) {
        r.status = AuthStatus::PolicyViolation;
        r.detail = "mTLS subject not mapped to a role";
        return r;
    }

    r.status = AuthStatus::Success;
    r.role   = role;
    r.detail = std::string("mtls:") + ctx.mtls_subject_cn;
    return r;
}

} // namespace m130::access

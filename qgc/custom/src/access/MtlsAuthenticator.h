#pragma once

#include "Authenticator.h"
#include "CertPolicy.h"

#include <memory>

namespace m130::access {

/// Authenticator that expects an already-validated client certificate — the
/// TLS layer populates the `AuthContext::mtls_subject_cn` field (and, in a
/// future revision, a full `X509CertFields` payload). If the subject CN
/// maps to a role via the policy, the authenticator returns `Success`.
///
/// This authenticator never reads passwords — it is intended to sit in the
/// chain alongside `LocalAuthenticator`/`LdapAuthenticator` as a first
/// attempt for operator consoles that present certificates.
class MtlsAuthenticator final : public IAuthenticator
{
public:
    explicit MtlsAuthenticator(std::shared_ptr<IChannelSecurity> channel)
        : _channel(std::move(channel)) {}

    std::string id() const override { return "mtls"; }
    AuthResult  authenticate(const AuthContext& ctx) override;

    /// Optional per-session cert payload. Set by the TLS glue code before
    /// the chain runs authenticate(). When unset, only the subject CN from
    /// the `AuthContext` is used for role lookup.
    void setPresentedCert(const X509CertFields& cert,
                          std::chrono::system_clock::time_point now);
    void clearPresentedCert();

private:
    std::shared_ptr<IChannelSecurity> _channel;
    bool                               _cert_presented = false;
    X509CertFields                     _cert;
    std::chrono::system_clock::time_point _cert_now{};
};

} // namespace m130::access

#pragma once

#include "Role.h"

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace m130::access {

/// Abstract X.509 certificate fields. Not a real cert parser — the values
/// are populated by whatever TLS layer we bind to (QSslCertificate,
/// OpenSSL, or a test fixture). Keeping the struct free of library types
/// means cert policy evaluation stays fully unit-testable.
struct X509CertFields {
    std::string                     subject_cn;
    std::string                     issuer_cn;
    std::vector<std::string>        subject_alt_names;      ///< DNS / URI / EMAIL
    std::vector<std::string>        extended_key_usages;    ///< OIDs or names
    std::string                     fingerprint_sha256_hex; ///< 64-char lowercase hex
    std::chrono::system_clock::time_point not_before{};
    std::chrono::system_clock::time_point not_after{};
    /// Human-readable issuer chain from leaf to root, subject CNs.
    std::vector<std::string>        issuer_chain;
};

/// Reasons a policy can reject a certificate.
enum class CertDecision : std::uint8_t {
    Allow                 = 0,
    Pinned                = 1,  ///< Allowed because its fingerprint is pinned
    NotPinned             = 2,  ///< Pinning enforced and fingerprint missing
    Expired               = 3,
    NotYetValid           = 4,
    SanMismatch           = 5,
    MissingEku            = 6,
    UntrustedIssuer       = 7,
    MissingFingerprint    = 8,
};

struct CertEvaluation {
    CertDecision decision = CertDecision::Allow;
    std::string  detail;
};

/// Configurable X.509 policy. Evaluation is pure — it does not touch the
/// network, filesystem or crypto libraries.
class CertPolicy
{
public:
    /// Pin the fingerprint as explicitly trusted. Hex is lowercased.
    void pinFingerprint(std::string_view sha256_hex);

    /// Require any pinned fingerprint to be present on the cert (otherwise
    /// cert is rejected even if otherwise valid). Off by default — pinning
    /// is strictly additive.
    void setRequirePinned(bool v) noexcept { _require_pinned = v; }

    /// Add a SAN that the cert MUST include (e.g. hostname of the GCS link
    /// peer). If none are set, any SAN is accepted.
    void addAllowedSan(std::string_view san);

    /// Add an EKU that MUST be present.
    void addRequiredEku(std::string_view eku);

    /// Issuer CN that must appear somewhere in the chain. Repeat calls
    /// produce a list — any one match is sufficient.
    void addTrustedIssuer(std::string_view issuer_cn);

    /// Map subject CN → application role. Used by `MtlsAuthenticator`.
    void mapRole(std::string_view subject_cn, Role role);

    /// Look up the mapped role for @p subject_cn. Returns `Role::None` on
    /// miss.
    Role lookupRole(std::string_view subject_cn) const;

    /// Evaluate @p cert at @p now.
    CertEvaluation evaluate(const X509CertFields& cert,
                            std::chrono::system_clock::time_point now) const;

private:
    std::unordered_set<std::string> _pinned_fingerprints;
    std::unordered_set<std::string> _allowed_sans;
    std::unordered_set<std::string> _required_ekus;
    std::unordered_set<std::string> _trusted_issuers;
    std::unordered_map<std::string, Role> _role_mapping;  // subject CN → role
    bool                            _require_pinned = false;
};

/// Abstract interface for applying a policy — separates the decision logic
/// from the caller's integration surface (e.g. `AccessController`).
class IChannelSecurity
{
public:
    virtual ~IChannelSecurity() = default;
    virtual CertEvaluation evaluate(const X509CertFields& cert,
                                    std::chrono::system_clock::time_point now) const = 0;
    /// Map a certificate's subject CN to an application role (Role::None
    /// when no mapping exists or mTLS authentication is not authorised for
    /// the subject).
    virtual Role lookupRole(std::string_view subject_cn) const = 0;
};

/// Thin adapter over `CertPolicy` satisfying `IChannelSecurity`.
class PolicyEnforcer final : public IChannelSecurity
{
public:
    explicit PolicyEnforcer(CertPolicy policy) : _policy(std::move(policy)) {}

    const CertPolicy& policy() const noexcept { return _policy; }
    CertPolicy&       policy()       noexcept { return _policy; }

    CertEvaluation evaluate(const X509CertFields& cert,
                            std::chrono::system_clock::time_point now) const override
    {
        return _policy.evaluate(cert, now);
    }

    Role lookupRole(std::string_view subject_cn) const override
    {
        return _policy.lookupRole(subject_cn);
    }

private:
    CertPolicy _policy;
};

} // namespace m130::access

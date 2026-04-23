#include "CertPolicy.h"

#include <algorithm>
#include <cctype>

namespace m130::access {

namespace {

std::string lower(std::string_view s)
{
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

bool iequals(std::string_view a, std::string_view b)
{
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i]))
            != std::tolower(static_cast<unsigned char>(b[i]))) return false;
    }
    return true;
}

} // namespace

void CertPolicy::pinFingerprint(std::string_view sha256_hex)
{
    _pinned_fingerprints.insert(lower(sha256_hex));
}

void CertPolicy::addAllowedSan(std::string_view san)
{
    _allowed_sans.insert(lower(san));
}

void CertPolicy::addRequiredEku(std::string_view eku)
{
    _required_ekus.insert(lower(eku));
}

void CertPolicy::addTrustedIssuer(std::string_view issuer_cn)
{
    _trusted_issuers.insert(lower(issuer_cn));
}

void CertPolicy::mapRole(std::string_view subject_cn, Role role)
{
    _role_mapping[lower(subject_cn)] = role;
}

Role CertPolicy::lookupRole(std::string_view subject_cn) const
{
    const auto it = _role_mapping.find(lower(subject_cn));
    return (it == _role_mapping.end()) ? Role::None : it->second;
}

CertEvaluation CertPolicy::evaluate(const X509CertFields& cert,
                                    std::chrono::system_clock::time_point now) const
{
    CertEvaluation out;

    // Validity window.
    if (cert.not_after.time_since_epoch().count() != 0 && now > cert.not_after) {
        out.decision = CertDecision::Expired;
        out.detail   = "certificate expired";
        return out;
    }
    if (cert.not_before.time_since_epoch().count() != 0 && now < cert.not_before) {
        out.decision = CertDecision::NotYetValid;
        out.detail   = "certificate not yet valid";
        return out;
    }

    // Fingerprint pinning.
    const std::string fp = lower(cert.fingerprint_sha256_hex);
    const bool has_pin_list = !_pinned_fingerprints.empty();
    const bool is_pinned = has_pin_list && _pinned_fingerprints.count(fp) > 0;
    if (_require_pinned) {
        if (fp.empty()) {
            out.decision = CertDecision::MissingFingerprint;
            out.detail   = "fingerprint missing and pinning required";
            return out;
        }
        if (!is_pinned) {
            out.decision = CertDecision::NotPinned;
            out.detail   = "fingerprint not in pin list";
            return out;
        }
    }

    // SAN allow-list.
    if (!_allowed_sans.empty()) {
        bool any = false;
        for (const auto& san : cert.subject_alt_names) {
            if (_allowed_sans.count(lower(san)) > 0) { any = true; break; }
        }
        if (!any) {
            out.decision = CertDecision::SanMismatch;
            out.detail   = "no SAN matched the allow-list";
            return out;
        }
    }

    // EKU requirements.
    if (!_required_ekus.empty()) {
        for (const auto& need : _required_ekus) {
            bool present = false;
            for (const auto& have : cert.extended_key_usages) {
                if (iequals(have, need)) { present = true; break; }
            }
            if (!present) {
                out.decision = CertDecision::MissingEku;
                out.detail   = "required EKU missing: " + need;
                return out;
            }
        }
    }

    // Trusted issuer chain.
    if (!_trusted_issuers.empty()) {
        bool any = false;
        auto probe = [&](const std::string& cn) {
            if (_trusted_issuers.count(lower(cn)) > 0) any = true;
        };
        probe(cert.issuer_cn);
        if (!any) for (const auto& link : cert.issuer_chain) probe(link);
        if (!any) {
            out.decision = CertDecision::UntrustedIssuer;
            out.detail   = "issuer not in trust list";
            return out;
        }
    }

    out.decision = is_pinned ? CertDecision::Pinned : CertDecision::Allow;
    return out;
}

} // namespace m130::access

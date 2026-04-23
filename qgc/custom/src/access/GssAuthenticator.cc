#include "GssAuthenticator.h"

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

std::string realmOf(std::string_view principal)
{
    const auto at = principal.rfind('@');
    if (at == std::string_view::npos) return {};
    return std::string(principal.substr(at + 1));
}

} // namespace

void FakeGssProvider::accept(std::string token, std::string principal)
{
    _principals[std::move(token)] = std::move(principal);
}

IGssProvider::AcceptResult
FakeGssProvider::acceptSecurityContext(std::string_view input_token)
{
    AcceptResult r;
    const auto it = _principals.find(std::string(input_token));
    if (it == _principals.end()) {
        r.error = "unknown token";
        return r;
    }
    r.complete     = true;
    r.principal    = it->second;
    r.output_token = "ack";
    return r;
}

void GssPolicy::mapPrincipal(std::string_view principal, Role role)
{
    _principal_map[lower(principal)] = role;
}

void GssPolicy::mapRealm(std::string_view realm, Role role)
{
    _realm_map[lower(realm)] = role;
}

Role GssPolicy::lookup(std::string_view principal) const
{
    const auto full = lower(principal);
    if (auto it = _principal_map.find(full); it != _principal_map.end()) {
        return it->second;
    }
    const auto realm = lower(realmOf(principal));
    if (!realm.empty()) {
        if (auto it = _realm_map.find(realm); it != _realm_map.end()) {
            return it->second;
        }
    }
    return Role::None;
}

AuthResult GssAuthenticator::authenticate(const AuthContext& ctx)
{
    AuthResult r;
    if (!_provider || ctx.gss_token.empty()) {
        r.status = AuthStatus::NotHandled;
        return r;
    }

    const auto res = _provider->acceptSecurityContext(ctx.gss_token);
    if (!res.error.empty()) {
        // Unknown / spoofed / expired token — treat as bad credential.
        r.status = AuthStatus::BadPassword;
        r.detail = res.error;
        return r;
    }
    if (!res.complete) {
        // Multi-leg SPNEGO — the caller needs to send another token.
        r.status = AuthStatus::NotHandled;
        r.detail = "multi-leg context not yet complete";
        return r;
    }
    const Role role = _policy.lookup(res.principal);
    if (role == Role::None) {
        r.status = AuthStatus::PolicyViolation;
        r.detail = "principal " + res.principal + " has no mapped role";
        return r;
    }

    r.status = AuthStatus::Success;
    r.role   = role;
    r.detail = "gssapi:" + res.principal;
    return r;
}

} // namespace m130::access

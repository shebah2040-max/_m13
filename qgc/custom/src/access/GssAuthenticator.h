#pragma once

#include "Authenticator.h"
#include "Role.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace m130::access {

/// Abstract GSSAPI / SPNEGO provider. Real implementations wrap
/// `libgssapi_krb5` (Linux/macOS) or Windows SSPI — none are linked in
/// the pure-C++ core. Applications that need Kerberos SSO inject a
/// concrete provider at construction.
class IGssProvider
{
public:
    struct AcceptResult {
        bool        complete = false;
        std::string output_token;   ///< base64-or-raw negotiate token to send back
        std::string principal;      ///< e.g. "alice@REALM.LOCAL" when complete
        std::string error;          ///< non-empty when the context is rejected
    };

    virtual ~IGssProvider() = default;
    virtual AcceptResult acceptSecurityContext(std::string_view input_token) = 0;
    virtual void         releaseContext() {}
};

/// Always-`NotHandled` provider. Installed by default so the chain stays
/// compilable without Kerberos support.
class StubGssProvider final : public IGssProvider
{
public:
    AcceptResult acceptSecurityContext(std::string_view) override
    { return AcceptResult{false, {}, {}, "gss provider not configured"}; }
};

/// Test/fixture provider backed by a prearranged token → principal map.
/// Produced tokens are opaque; callers may set them to any non-empty
/// string.
class FakeGssProvider final : public IGssProvider
{
public:
    void accept(std::string token, std::string principal);
    AcceptResult acceptSecurityContext(std::string_view input_token) override;

private:
    std::unordered_map<std::string, std::string> _principals; // token → principal
};

/// Policy mapping principals (or realms) to application roles.
class GssPolicy
{
public:
    void mapPrincipal(std::string_view principal, Role role);
    void mapRealm(std::string_view realm, Role role);
    Role lookup(std::string_view principal) const;

private:
    std::unordered_map<std::string, Role> _principal_map;
    std::unordered_map<std::string, Role> _realm_map;
};

/// `IAuthenticator` that delegates to an `IGssProvider` and maps the
/// resolved principal to an application role via `GssPolicy`.
class GssAuthenticator final : public IAuthenticator
{
public:
    GssAuthenticator(std::shared_ptr<IGssProvider> provider, GssPolicy policy)
        : _provider(std::move(provider)), _policy(std::move(policy)) {}

    std::string id() const override { return "gssapi"; }
    AuthResult  authenticate(const AuthContext& ctx) override;

private:
    std::shared_ptr<IGssProvider> _provider;
    GssPolicy                     _policy;
};

} // namespace m130::access

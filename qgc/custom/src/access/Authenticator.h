#pragma once

#include "Role.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace m130::access {

class UserManager;

struct AuthContext {
    std::string user_id;
    std::string password;
    std::string totp_code;        ///< empty if not provided
    std::string mtls_subject_cn;  ///< populated by the channel layer, empty otherwise
};

enum class AuthStatus : std::uint8_t {
    Success          = 0,
    UnknownUser      = 1,
    BadPassword      = 2,
    AccountLocked    = 3,
    RequiresTotp     = 4,
    BadTotp          = 5,
    NotHandled       = 6,  ///< Pass to the next authenticator in the chain
    PolicyViolation  = 7,
};

struct AuthResult {
    AuthStatus  status = AuthStatus::NotHandled;
    Role        role = Role::None;
    std::string detail;
};

/// Pluggable authenticator — one implementation per backend (local, LDAP,
/// mTLS cert-based, …). The `AuthenticatorChain` walks them in order,
/// returning the first non-NotHandled result.
class IAuthenticator
{
public:
    virtual ~IAuthenticator() = default;
    virtual std::string id() const = 0;
    virtual AuthResult  authenticate(const AuthContext& ctx) = 0;
};

/// Wraps `UserManager` as an `IAuthenticator`. This is the default M130 GCS
/// authenticator — password + optional TOTP against the local user store.
class LocalAuthenticator final : public IAuthenticator
{
public:
    explicit LocalAuthenticator(UserManager* um) noexcept : _um(um) {}
    std::string id() const override { return "local"; }
    AuthResult  authenticate(const AuthContext& ctx) override;
private:
    UserManager* _um = nullptr;
};

/// Ordered chain of authenticators. Default construction is empty; callers
/// `add()` backends in priority order.
class AuthenticatorChain
{
public:
    void add(std::shared_ptr<IAuthenticator> backend);
    void clear() noexcept { _backends.clear(); }
    std::size_t size() const noexcept { return _backends.size(); }

    /// Walk the chain. Returns the first `status != NotHandled` result, or a
    /// final `NotHandled` if no backend owned the request.
    AuthResult authenticate(const AuthContext& ctx);

    /// Names of registered backends, for diagnostics.
    std::vector<std::string> backendIds() const;

private:
    std::vector<std::shared_ptr<IAuthenticator>> _backends;
};

} // namespace m130::access

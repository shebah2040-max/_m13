#include "Authenticator.h"

#include "UserManager.h"

namespace m130::access {

AuthResult LocalAuthenticator::authenticate(const AuthContext& ctx)
{
    AuthResult r;
    if (!_um) {
        r.status = AuthStatus::NotHandled;
        r.detail = "no user manager";
        return r;
    }

    auto rec = _um->find(ctx.user_id);
    if (!rec) {
        r.status = AuthStatus::UnknownUser;
        r.detail = "no such user";
        return r;
    }

    const auto first = _um->attemptPassword(ctx.user_id, ctx.password);
    switch (first.result) {
    case LoginResult::UnknownUser:   r.status = AuthStatus::UnknownUser;   r.detail = first.detail; return r;
    case LoginResult::BadPassword:   r.status = AuthStatus::BadPassword;   r.detail = first.detail; return r;
    case LoginResult::AccountLocked: r.status = AuthStatus::AccountLocked; r.detail = first.detail; return r;
    case LoginResult::WeakPassword:  r.status = AuthStatus::PolicyViolation; r.detail = first.detail; return r;
    case LoginResult::Success:
        r.status = AuthStatus::Success;
        r.role   = rec->role;
        return r;
    case LoginResult::RequiresTotp:
        if (ctx.totp_code.empty()) {
            r.status = AuthStatus::RequiresTotp;
            r.detail = "totp code required";
            return r;
        }
        if (!_um->verifyTotp(ctx.user_id, ctx.totp_code)) {
            r.status = AuthStatus::BadTotp;
            r.detail = "totp mismatch";
            return r;
        }
        r.status = AuthStatus::Success;
        r.role   = rec->role;
        return r;
    case LoginResult::BadTotp:
        r.status = AuthStatus::BadTotp;
        r.detail = first.detail;
        return r;
    }
    r.status = AuthStatus::NotHandled;
    return r;
}

void AuthenticatorChain::add(std::shared_ptr<IAuthenticator> backend)
{
    if (backend) _backends.push_back(std::move(backend));
}

AuthResult AuthenticatorChain::authenticate(const AuthContext& ctx)
{
    for (auto& b : _backends) {
        auto r = b->authenticate(ctx);
        if (r.status != AuthStatus::NotHandled) return r;
    }
    return { AuthStatus::NotHandled, Role::None, "no backend handled the request" };
}

std::vector<std::string> AuthenticatorChain::backendIds() const
{
    std::vector<std::string> out;
    out.reserve(_backends.size());
    for (const auto& b : _backends) out.push_back(b->id());
    return out;
}

} // namespace m130::access

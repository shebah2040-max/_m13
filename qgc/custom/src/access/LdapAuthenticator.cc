#include "LdapAuthenticator.h"

#include "LdapDn.h"

#include <algorithm>
#include <cctype>

namespace m130::access {

namespace {

bool caseEq(std::string_view a, std::string_view b)
{
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i]))
            != std::tolower(static_cast<unsigned char>(b[i]))) return false;
    }
    return true;
}

std::string substituteUser(std::string_view templ, std::string_view user)
{
    std::string out;
    out.reserve(templ.size() + user.size());
    const std::string_view tok = "{user}";
    std::size_t i = 0;
    while (i < templ.size()) {
        if (i + tok.size() <= templ.size() && templ.substr(i, tok.size()) == tok) {
            out.append(user);
            i += tok.size();
        } else {
            out.push_back(templ[i]);
            ++i;
        }
    }
    return out;
}

std::string canonDn(std::string_view s)
{
    const auto dn = LdapDn::parse(s);
    return dn ? dn->canonical() : std::string();
}

} // namespace

// ---------------- InMemoryLdap ----------------

void InMemoryLdap::add(std::string dn, Entry e)
{
    _entries[canonDn(dn)] = std::move(e);
}

bool InMemoryLdap::bind(std::string_view dn, std::string_view password)
{
    const auto it = _entries.find(canonDn(dn));
    if (it == _entries.end()) return false;
    // Empty passwords are always rejected (anonymous binds disabled).
    if (password.empty()) return false;
    return it->second.password == password;
}

std::vector<LdapEntry> InMemoryLdap::search(std::string_view base_dn,
                                            const LdapFilter& filter)
{
    std::vector<LdapEntry> out;
    const std::string base = canonDn(base_dn);
    for (const auto& [dn, data] : _entries) {
        // Cheap suffix match — the DN must end with the base. An entry with a
        // DN shorter than the base is outside the subtree and is skipped.
        if (!base.empty()) {
            if (dn.size() < base.size()) continue;
            const std::string suffix = dn.substr(dn.size() - base.size());
            if (!caseEq(suffix, base)) continue;
        }

        LdapEntry e;
        e.add("dn", dn);
        for (const auto& [attr, values] : data.attrs) {
            for (const auto& v : values) e.add(attr, v);
        }
        if (filter.matches(e)) out.push_back(std::move(e));
    }
    return out;
}

// ---------------- LdapAuthenticator ----------------

AuthResult LdapAuthenticator::authenticate(const AuthContext& ctx)
{
    AuthResult r;
    if (ctx.user_id.empty()) {
        r.status = AuthStatus::NotHandled;
        return r;
    }

    const std::string bind_dn = _cfg.user_dn_template.empty()
        ? ctx.user_id
        : substituteUser(_cfg.user_dn_template, ctx.user_id);

    if (!_transport) {
        r.status = AuthStatus::PolicyViolation;
        r.detail = "LDAP transport not configured";
        return r;
    }
    if (!_transport->bind(bind_dn, ctx.password)) {
        r.status = AuthStatus::BadPassword;
        r.detail = "LDAP bind failed";
        return r;
    }

    // Look up the user entry for group membership.
    auto filt = LdapFilter::parse("(objectClass=*)");
    std::vector<LdapEntry> hits;
    if (filt) hits = _transport->search(bind_dn, *filt);

    auto findMemberships = [&](const LdapEntry& e) {
        return e.values("memberof");
    };

    Role assigned = _cfg.fallback_role;
    for (const auto& rule : _cfg.role_rules) {
        for (const auto& e : hits) {
            const auto* groups = findMemberships(e);
            if (!groups) continue;
            for (const auto& g : *groups) {
                if (caseEq(canonDn(g), canonDn(rule.group_dn))) {
                    assigned = rule.role;
                    goto done;
                }
            }
        }
    }
done:
    if (assigned == Role::None) {
        r.status = AuthStatus::PolicyViolation;
        r.detail = "User has no mapped role";
        return r;
    }
    r.status = AuthStatus::Success;
    r.role   = assigned;
    r.detail = bind_dn;
    return r;
}

} // namespace m130::access

#pragma once

#include "Authenticator.h"
#include "LdapFilter.h"
#include "Role.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace m130::access {

/// Pluggable LDAP transport. The authenticator calls this with a DN and a
/// password to validate the bind, and with a filter to look up group
/// membership. Concrete implementations may wrap OpenLDAP, Active
/// Directory, or in-memory fixtures for tests.
class ILdapTransport
{
public:
    virtual ~ILdapTransport() = default;

    /// Attempt a simple bind with @p dn and @p password. Returns true on
    /// success. The DN will typically be formed from a `user_dn_template`
    /// like `uid={user},ou=People,dc=m130,dc=local`.
    virtual bool bind(std::string_view dn, std::string_view password) = 0;

    /// Return entries matching @p filter under @p base_dn. The returned
    /// entries MUST populate the attributes the caller needs
    /// (typically `memberOf`, `cn`, `uid`).
    virtual std::vector<LdapEntry> search(std::string_view base_dn,
                                          const LdapFilter& filter) = 0;
};

/// In-memory LDAP fixture used by tests. Stores password and attributes
/// per DN. Pre-populated entries resolve for both bind and search.
class InMemoryLdap final : public ILdapTransport
{
public:
    struct Entry {
        std::string                              password;
        std::unordered_map<std::string, std::vector<std::string>> attrs;
    };

    /// Add or overwrite an entry.
    void add(std::string dn, Entry e);

    bool bind(std::string_view dn, std::string_view password) override;
    std::vector<LdapEntry> search(std::string_view base_dn,
                                  const LdapFilter& filter) override;

private:
    // Key is canonical DN (lower-cased attribute names, escape-normalised).
    std::unordered_map<std::string, Entry> _entries;
};

/// Mapping rule: if the LDAP entry's `memberOf` contains a given group DN,
/// assign the specified role. Evaluated top-to-bottom; first match wins.
struct LdapRoleRule {
    std::string group_dn;
    Role        role = Role::Observer;
};

struct LdapConfig {
    /// DN template substituting `{user}` with the username. If empty, the
    /// username is used as the bind DN verbatim.
    std::string                user_dn_template = "uid={user},ou=People,dc=m130,dc=local";
    std::string                group_base_dn    = "ou=Groups,dc=m130,dc=local";
    std::vector<LdapRoleRule>  role_rules;
    /// Default role when no rule matches (None = reject).
    Role                       fallback_role    = Role::None;
};

/// LDAP-based authenticator. Binds the user, then derives a role from group
/// membership via the configured rules.
class LdapAuthenticator final : public IAuthenticator
{
public:
    LdapAuthenticator(std::shared_ptr<ILdapTransport> transport, LdapConfig cfg)
        : _transport(std::move(transport)), _cfg(std::move(cfg)) {}

    std::string id() const override { return "ldap"; }
    AuthResult  authenticate(const AuthContext& ctx) override;

    const LdapConfig& config() const noexcept { return _cfg; }

private:
    std::shared_ptr<ILdapTransport> _transport;
    LdapConfig                      _cfg;
};

} // namespace m130::access

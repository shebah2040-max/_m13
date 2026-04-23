#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace m130::access {

/// Generic attribute bag used as the subject of a filter evaluation. A key
/// maps to a list of values (LDAP attributes are multi-valued). Lookups
/// are case-insensitive on the attribute name.
class LdapEntry
{
public:
    void add(std::string attr, std::string value);

    /// Case-insensitive attribute lookup.
    const std::vector<std::string>* values(std::string_view attr) const;

private:
    // Key is the lower-cased attribute name.
    std::unordered_map<std::string, std::vector<std::string>> _attrs;
};

/// Parsed RFC 4515 LDAP search filter.
///
/// Supports: `(&f1 f2 …)`, `(|f1 f2 …)`, `(!f)`, `(attr=value)`, `(attr=*)`
/// and the equality wildcard. Extensible match and `~=`/`<=`/`>=` are
/// rejected — they are not used by the M130 GCS policy grammar. Values are
/// compared case-insensitively.
class LdapFilter
{
public:
    enum class Op : std::uint8_t { And, Or, Not, Equals, Present };

    static std::optional<LdapFilter> parse(std::string_view s);

    /// Evaluate against an attribute entry.
    bool matches(const LdapEntry& e) const;

    Op op() const noexcept { return _op; }

private:
    Op _op = Op::And;
    std::string _attr;        ///< populated for Equals/Present
    std::string _value;       ///< populated for Equals
    std::vector<std::shared_ptr<LdapFilter>> _children;

    static std::optional<std::pair<LdapFilter, std::size_t>>
    _parse(std::string_view s, std::size_t pos);
};

} // namespace m130::access

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace m130::access {

/// Single attribute/value pair inside a DN relative distinguished name
/// (a DN RDN can hold several of these joined by `+`, which we flatten).
struct DnPair {
    std::string name;   ///< Attribute name, case-folded to lowercase.
    std::string value;  ///< Value with `\` escapes resolved.
};

/// Parsed LDAP distinguished name per RFC 4514.
///
/// This implementation is intentionally small — enough to validate DN shape,
/// extract attributes (typically `uid`, `cn`, `ou`, `dc`) and reconstruct a
/// normalised DN for comparison. It does **not** implement the full escape
/// grammar (hex escapes, leading/trailing space handling are handled; the
/// compact subset used by the M130 GCS schema is covered).
class LdapDn
{
public:
    /// Parse @p s as a DN. Returns `std::nullopt` on malformed input.
    static std::optional<LdapDn> parse(std::string_view s);

    const std::vector<DnPair>& pairs() const noexcept { return _pairs; }

    /// Lookup the first value for @p attr (case-insensitive). Empty if
    /// absent.
    std::string find(std::string_view attr) const;

    /// Canonicalised serialisation (`cn=foo,ou=bar,dc=baz` with attribute
    /// names lower-cased and values unescaped). Useful as a map key.
    std::string canonical() const;

    bool empty() const noexcept { return _pairs.empty(); }

private:
    std::vector<DnPair> _pairs;
};

} // namespace m130::access

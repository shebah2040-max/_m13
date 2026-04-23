#include "LdapDn.h"

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

bool caseEq(std::string_view a, std::string_view b)
{
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i]))
            != std::tolower(static_cast<unsigned char>(b[i]))) return false;
    }
    return true;
}

bool unescapeValue(std::string_view raw, std::string& out)
{
    out.clear();
    out.reserve(raw.size());
    for (std::size_t i = 0; i < raw.size(); ++i) {
        const char c = raw[i];
        if (c != '\\') { out.push_back(c); continue; }
        if (i + 1 >= raw.size()) return false;
        const char n = raw[i + 1];
        // Two-char hex escape (RFC 4514 §2.4).
        auto hex = [](char ch) -> int {
            if (ch >= '0' && ch <= '9') return ch - '0';
            if (ch >= 'a' && ch <= 'f') return 10 + (ch - 'a');
            if (ch >= 'A' && ch <= 'F') return 10 + (ch - 'A');
            return -1;
        };
        const int hi = hex(n);
        if (hi >= 0 && i + 2 < raw.size()) {
            const int lo = hex(raw[i + 2]);
            if (lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
                continue;
            }
        }
        // Literal escape of a special char.
        out.push_back(n);
        ++i;
    }
    // Trim leading/trailing unescaped spaces (RFC 4514).
    while (!out.empty() && out.front() == ' ') out.erase(out.begin());
    while (!out.empty() && out.back()  == ' ') out.pop_back();
    return true;
}

// Split a DN string on top-level commas (respecting `\,` escapes).
std::vector<std::string_view> splitRdns(std::string_view s)
{
    std::vector<std::string_view> out;
    std::size_t start = 0;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\') { ++i; continue; }
        if (s[i] == ',') {
            out.push_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    if (start <= s.size()) out.push_back(s.substr(start));
    return out;
}

} // namespace

std::optional<LdapDn> LdapDn::parse(std::string_view s)
{
    LdapDn dn;
    if (s.empty()) return dn; // empty DN is the root.

    for (const auto& rdn : splitRdns(s)) {
        // Each RDN may be `attr=val` or `attr1=val1+attr2=val2` (flattened).
        std::size_t start = 0;
        for (std::size_t i = 0; i <= rdn.size(); ++i) {
            const bool boundary = (i == rdn.size())
                || (rdn[i] == '+' && (i == 0 || rdn[i - 1] != '\\'));
            if (!boundary) continue;

            const auto piece = rdn.substr(start, i - start);
            const auto eq = piece.find('=');
            if (eq == std::string_view::npos || eq == 0) return std::nullopt;

            DnPair p;
            p.name = lower(piece.substr(0, eq));
            // Trim whitespace inside attribute name.
            while (!p.name.empty() && p.name.back()  == ' ') p.name.pop_back();
            while (!p.name.empty() && p.name.front() == ' ') p.name.erase(p.name.begin());
            if (p.name.empty()) return std::nullopt;

            if (!unescapeValue(piece.substr(eq + 1), p.value)) return std::nullopt;
            dn._pairs.push_back(std::move(p));
            start = i + 1;
        }
    }
    return dn;
}

std::string LdapDn::find(std::string_view attr) const
{
    for (const auto& p : _pairs) {
        if (caseEq(p.name, attr)) return p.value;
    }
    return {};
}

std::string LdapDn::canonical() const
{
    std::string out;
    bool first = true;
    for (const auto& p : _pairs) {
        if (!first) out.push_back(',');
        first = false;
        out += p.name;
        out.push_back('=');
        for (char c : p.value) {
            // Escape LDAP special chars for round-trip stability.
            if (c == ',' || c == '+' || c == '"' || c == '\\' || c == '<' || c == '>' || c == ';') {
                out.push_back('\\');
            }
            out.push_back(c);
        }
    }
    return out;
}

} // namespace m130::access

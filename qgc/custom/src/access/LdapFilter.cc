#include "LdapFilter.h"

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
} // namespace

// ---------------- LdapEntry ----------------

void LdapEntry::add(std::string attr, std::string value)
{
    _attrs[lower(attr)].push_back(std::move(value));
}

const std::vector<std::string>* LdapEntry::values(std::string_view attr) const
{
    const auto it = _attrs.find(lower(attr));
    return (it == _attrs.end()) ? nullptr : &it->second;
}

// ---------------- LdapFilter ----------------

std::optional<std::pair<LdapFilter, std::size_t>>
LdapFilter::_parse(std::string_view s, std::size_t pos)
{
    if (pos >= s.size() || s[pos] != '(') return std::nullopt;
    ++pos;
    if (pos >= s.size()) return std::nullopt;

    LdapFilter f;
    const char head = s[pos];

    auto parseChildren = [&](Op op) -> std::optional<std::pair<LdapFilter, std::size_t>> {
        f._op = op;
        ++pos; // consume operator char
        while (pos < s.size() && s[pos] != ')') {
            auto child = _parse(s, pos);
            if (!child) return std::nullopt;
            f._children.push_back(std::make_shared<LdapFilter>(std::move(child->first)));
            pos = child->second;
        }
        if (pos >= s.size() || s[pos] != ')') return std::nullopt;
        return std::make_pair(std::move(f), pos + 1);
    };

    if (head == '&') return parseChildren(Op::And);
    if (head == '|') return parseChildren(Op::Or);
    if (head == '!') {
        f._op = Op::Not;
        ++pos;
        auto child = _parse(s, pos);
        if (!child) return std::nullopt;
        f._children.push_back(std::make_shared<LdapFilter>(std::move(child->first)));
        pos = child->second;
        if (pos >= s.size() || s[pos] != ')') return std::nullopt;
        return std::make_pair(std::move(f), pos + 1);
    }

    // Equality / presence leaf: <attr>=<value> up to ')'.
    std::size_t eq = s.find('=', pos);
    if (eq == std::string_view::npos || eq > s.size()) return std::nullopt;
    std::size_t close = s.find(')', eq + 1);
    if (close == std::string_view::npos) return std::nullopt;

    // Reject ~=, <=, >= and the extensible variants.
    if (eq > 0 && (s[eq - 1] == '~' || s[eq - 1] == '<' || s[eq - 1] == '>')) {
        return std::nullopt;
    }

    const auto attr = s.substr(pos, eq - pos);
    const auto val  = s.substr(eq + 1, close - eq - 1);
    if (attr.empty()) return std::nullopt;

    f._attr = std::string(attr);
    if (val == "*") {
        f._op = Op::Present;
    } else {
        f._op = Op::Equals;
        f._value = std::string(val);
    }
    return std::make_pair(std::move(f), close + 1);
}

std::optional<LdapFilter> LdapFilter::parse(std::string_view s)
{
    auto r = _parse(s, 0);
    if (!r) return std::nullopt;
    if (r->second != s.size()) return std::nullopt;
    return std::move(r->first);
}

bool LdapFilter::matches(const LdapEntry& e) const
{
    switch (_op) {
    case Op::And:
        for (const auto& c : _children) if (!c->matches(e)) return false;
        return true;
    case Op::Or:
        for (const auto& c : _children) if (c->matches(e))  return true;
        return false;
    case Op::Not:
        return !_children.empty() && !_children.front()->matches(e);
    case Op::Present: {
        const auto* v = e.values(_attr);
        return v && !v->empty();
    }
    case Op::Equals: {
        const auto* v = e.values(_attr);
        if (!v) return false;
        for (const auto& val : *v) if (caseEq(val, _value)) return true;
        return false;
    }
    }
    return false;
}

} // namespace m130::access

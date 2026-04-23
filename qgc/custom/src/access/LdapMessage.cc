#include "LdapMessage.h"

namespace m130::access::ldap {

namespace {

using ber::Encoder;
using ber::Decoder;
using ber::Class;

constexpr std::uint8_t applicationIdent(std::uint8_t number, bool constructed = true)
{
    return ber::identifier(Class::Application, constructed, number);
}
constexpr std::uint8_t contextIdent(std::uint8_t number, bool constructed = false)
{
    return ber::identifier(Class::ContextSpecific, constructed, number);
}

void encodeFilterInto(Encoder& e, const LdapFilter& f)
{
    using Op = LdapFilter::Op;
    switch (f.op()) {
    case Op::And: {
        const auto idx = e.startConstructed(contextIdent(0, true));
        for (const auto& c : f.children()) encodeFilterInto(e, *c);
        e.closeConstructed(idx);
        break;
    }
    case Op::Or: {
        const auto idx = e.startConstructed(contextIdent(1, true));
        for (const auto& c : f.children()) encodeFilterInto(e, *c);
        e.closeConstructed(idx);
        break;
    }
    case Op::Not: {
        const auto idx = e.startConstructed(contextIdent(2, true));
        if (!f.children().empty()) encodeFilterInto(e, *f.children().front());
        e.closeConstructed(idx);
        break;
    }
    case Op::Equals: {
        const auto idx = e.startConstructed(contextIdent(3, true));
        e.writeOctetString(f.attribute());
        e.writeOctetString(f.value());
        e.closeConstructed(idx);
        break;
    }
    case Op::Present:
        e.writeRaw(contextIdent(7), f.attribute());
        break;
    }
}

ResultCode codeFromInt(std::int64_t v)
{
    switch (v) {
    case 0:  return ResultCode::Success;
    case 1:  return ResultCode::OperationsError;
    case 2:  return ResultCode::ProtocolError;
    case 49: return ResultCode::InvalidCredentials;
    case 53: return ResultCode::UnwillingToPerform;
    default: return ResultCode::Unknown;
    }
}

bool decodeLdapResult(Decoder& d, ResultCode& code, std::string& matched, std::string& diag)
{
    auto ec = d.readEnum();
    if (!ec) return false;
    auto m  = d.readOctetString();
    if (!m)  return false;
    auto di = d.readOctetString();
    if (!di) return false;
    code    = codeFromInt(*ec);
    matched = *m;
    diag    = *di;
    return true;
}

} // namespace

std::vector<std::uint8_t> encodeFilter(const LdapFilter& filter)
{
    Encoder e;
    encodeFilterInto(e, filter);
    return e.release();
}

std::vector<std::uint8_t> encodeBindRequest(const BindRequest& req)
{
    Encoder e;
    const auto outer = e.startConstructed(ber::tag::kSequence);
    e.writeInt(req.message_id);

    // [APPLICATION 0] BindRequest
    const auto bind = e.startConstructed(applicationIdent(op::kBindRequest));
    e.writeInt(req.version);
    e.writeOctetString(req.dn);
    // AuthenticationChoice::simple [0] OCTET STRING
    e.writeRaw(contextIdent(0, false), req.password);
    e.closeConstructed(bind);

    e.closeConstructed(outer);
    return e.release();
}

std::vector<std::uint8_t> encodeSearchRequest(const SearchRequest& req)
{
    Encoder e;
    const auto outer = e.startConstructed(ber::tag::kSequence);
    e.writeInt(req.message_id);

    // [APPLICATION 3] SearchRequest
    const auto sr = e.startConstructed(applicationIdent(op::kSearchRequest));
    e.writeOctetString(req.base_dn);
    e.writeEnum(static_cast<std::int64_t>(req.scope));
    e.writeEnum(0); // derefAliases = neverDerefAliases
    e.writeInt(req.size_limit);
    e.writeInt(req.time_limit);
    e.writeBool(req.types_only);

    encodeFilterInto(e, req.filter);

    // AttributeSelection ::= SEQUENCE OF LDAPString
    const auto attrs = e.startConstructed(ber::tag::kSequence);
    for (const auto& a : req.attributes) e.writeOctetString(a);
    e.closeConstructed(attrs);

    e.closeConstructed(sr);
    e.closeConstructed(outer);
    return e.release();
}

std::vector<std::uint8_t> encodeUnbindRequest(std::int64_t message_id)
{
    Encoder e;
    const auto outer = e.startConstructed(ber::tag::kSequence);
    e.writeInt(message_id);
    // UnbindRequest ::= [APPLICATION 2] NULL
    e.writeRaw(applicationIdent(op::kUnbindRequest, false), std::string_view{});
    e.closeConstructed(outer);
    return e.release();
}

std::optional<AnyResponse> decodeMessage(const std::vector<std::uint8_t>& data,
                                         std::size_t& consumed)
{
    consumed = 0;
    Decoder top(data);
    const std::size_t start = top.position();

    auto envelope = top.readConstructed(ber::tag::kSequence);
    if (!envelope) return std::nullopt;

    auto id = envelope->readInt();
    if (!id) return std::nullopt;

    std::uint8_t ident = 0;
    const std::uint8_t* value = nullptr;
    std::size_t len = 0;
    if (!envelope->next(ident, value, len)) return std::nullopt;

    AnyResponse out;
    out.message_id = *id;

    const std::uint8_t number = ident & 0x1F;
    out.op = number;

    Decoder inner(value, len);

    if (number == op::kBindResponse) {
        out.bind.message_id = *id;
        if (!decodeLdapResult(inner, out.bind.result_code,
                              out.bind.matched_dn, out.bind.diagnostic))
            return std::nullopt;
    } else if (number == op::kSearchResultEntry) {
        out.entry.message_id = *id;
        auto dn = inner.readOctetString();
        if (!dn) return std::nullopt;
        out.entry.dn = *dn;
        auto attr_list = inner.readConstructed(ber::tag::kSequence);
        if (!attr_list) return std::nullopt;
        while (!attr_list->atEnd()) {
            auto pair = attr_list->readConstructed(ber::tag::kSequence);
            if (!pair) return std::nullopt;
            auto name = pair->readOctetString();
            if (!name) return std::nullopt;
            auto vals = pair->readConstructed(ber::tag::kSet);
            if (!vals) return std::nullopt;
            auto& bucket = out.entry.attrs[*name];
            while (!vals->atEnd()) {
                auto v = vals->readOctetString();
                if (!v) return std::nullopt;
                bucket.push_back(*v);
            }
        }
    } else if (number == op::kSearchResultDone) {
        out.done.message_id = *id;
        if (!decodeLdapResult(inner, out.done.result_code,
                              out.done.matched_dn, out.done.diagnostic))
            return std::nullopt;
    } else {
        return std::nullopt;
    }

    consumed = top.position() - start;
    return out;
}

} // namespace m130::access::ldap

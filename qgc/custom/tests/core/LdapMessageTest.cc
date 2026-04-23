#include "access/LdapMessage.h"
#include "access/Ber.h"
#include "test_support.h"

#include <cstring>

using namespace m130::access;
using namespace m130::access::ldap;

namespace {

std::vector<std::uint8_t> hex(std::initializer_list<int> bytes)
{
    std::vector<std::uint8_t> out;
    out.reserve(bytes.size());
    for (int b : bytes) out.push_back(static_cast<std::uint8_t>(b));
    return out;
}

} // namespace

int encodesBindRequestStructure()
{
    BindRequest req;
    req.message_id = 1;
    req.version    = 3;
    req.dn         = "cn=admin,dc=m130,dc=local";
    req.password   = "secret";

    const auto blob = encodeBindRequest(req);

    // First byte is SEQUENCE tag.
    M130_REQUIRE(!blob.empty());
    M130_REQUIRE(blob.front() == ber::tag::kSequence);

    // Decode and make sure the nested application tag is [APPLICATION 0].
    ber::Decoder top(blob);
    auto env = top.readConstructed(ber::tag::kSequence);
    M130_REQUIRE(env.has_value());
    auto id = env->readInt();
    M130_REQUIRE(id && *id == 1);
    const auto app0 = ber::identifier(ber::Class::Application, true, 0);
    auto bind = env->readConstructed(app0);
    M130_REQUIRE(bind.has_value());
    auto ver = bind->readInt();
    auto dn  = bind->readOctetString();
    M130_REQUIRE(ver && *ver == 3);
    M130_REQUIRE(dn  && *dn  == "cn=admin,dc=m130,dc=local");
    return 0;
}

int decodesBindResponseSuccess()
{
    // Hand-crafted BindResponse with resultCode=success, matchedDN="", diag="".
    //   0x30 0x0c                                  SEQUENCE len=12
    //     0x02 0x01 0x01                            messageID=1
    //     0x61 0x07                                 [APPLICATION 1] SEQUENCE len=7
    //       0x0A 0x01 0x00                            resultCode = 0
    //       0x04 0x00                                 matchedDN = ""
    //       0x04 0x00                                 diagnostic = ""
    auto blob = hex({
        0x30, 0x0C,
            0x02, 0x01, 0x01,
            0x61, 0x07,
                0x0A, 0x01, 0x00,
                0x04, 0x00,
                0x04, 0x00,
    });
    std::size_t consumed = 0;
    auto resp = decodeMessage(blob, consumed);
    M130_REQUIRE(resp.has_value());
    M130_REQUIRE(consumed == blob.size());
    M130_REQUIRE(resp->op == op::kBindResponse);
    M130_REQUIRE(resp->bind.message_id == 1);
    M130_REQUIRE(resp->bind.result_code == ResultCode::Success);
    return 0;
}

int decodesBindResponseInvalidCredentials()
{
    auto blob = hex({
        0x30, 0x11,
            0x02, 0x01, 0x02,
            0x61, 0x0C,
                0x0A, 0x01, 0x31,      // resultCode = 49 (invalidCredentials)
                0x04, 0x00,
                0x04, 0x05, 'w','r','o','n','g',
    });
    std::size_t consumed = 0;
    auto resp = decodeMessage(blob, consumed);
    M130_REQUIRE(resp.has_value());
    M130_REQUIRE(resp->op == op::kBindResponse);
    M130_REQUIRE(resp->bind.result_code == ResultCode::InvalidCredentials);
    M130_REQUIRE(resp->bind.diagnostic == "wrong");
    return 0;
}

int decodesSearchResultEntry()
{
    // SearchResultEntry for uid=alice with attribute "memberOf"=["cn=ops"].
    //   0x30 len
    //     0x02 0x01 0x02                            messageID=2
    //     0x64 len                                  [APPLICATION 4]
    //       OCTET STRING dn
    //       SEQUENCE (attributes)
    //         SEQUENCE
    //           OCTET STRING "memberOf"
    //           SET
    //             OCTET STRING "cn=ops"
    std::string dn = "uid=alice,ou=People,dc=m130,dc=local";
    std::string attr = "memberOf";
    std::string val  = "cn=ops";

    ber::Encoder e;
    const auto env = e.startConstructed(ber::tag::kSequence);
    e.writeInt(2);
    const auto entry = e.startConstructed(ber::identifier(ber::Class::Application, true, 4));
    e.writeOctetString(dn);
    const auto attrs = e.startConstructed(ber::tag::kSequence);
    const auto pair  = e.startConstructed(ber::tag::kSequence);
    e.writeOctetString(attr);
    const auto set = e.startConstructed(ber::tag::kSet);
    e.writeOctetString(val);
    e.closeConstructed(set);
    e.closeConstructed(pair);
    e.closeConstructed(attrs);
    e.closeConstructed(entry);
    e.closeConstructed(env);

    std::size_t consumed = 0;
    auto resp = decodeMessage(e.bytes(), consumed);
    M130_REQUIRE(resp.has_value());
    M130_REQUIRE(resp->op == op::kSearchResultEntry);
    M130_REQUIRE(resp->entry.dn == dn);
    M130_REQUIRE(resp->entry.attrs.at("memberOf").size() == 1);
    M130_REQUIRE(resp->entry.attrs.at("memberOf").front() == val);
    return 0;
}

int encodesSearchRequestWithFilter()
{
    auto filt = LdapFilter::parse("(&(objectClass=inetOrgPerson)(uid=alice))");
    M130_REQUIRE(filt.has_value());

    SearchRequest req;
    req.message_id = 3;
    req.base_dn    = "ou=People,dc=m130,dc=local";
    req.scope      = Scope::WholeSubtree;
    req.filter     = *filt;
    req.attributes = {"dn", "memberOf"};

    auto blob = encodeSearchRequest(req);
    M130_REQUIRE(!blob.empty());

    ber::Decoder top(blob);
    auto env = top.readConstructed(ber::tag::kSequence);
    M130_REQUIRE(env.has_value());
    auto id = env->readInt();
    M130_REQUIRE(id && *id == 3);
    auto sr = env->readConstructed(ber::identifier(ber::Class::Application, true, 3));
    M130_REQUIRE(sr.has_value());
    auto base = sr->readOctetString();
    auto scope = sr->readEnum();
    auto deref = sr->readEnum();
    auto szl   = sr->readInt();
    auto tlim  = sr->readInt();
    auto tonly = sr->readBool();
    M130_REQUIRE(base && *base == "ou=People,dc=m130,dc=local");
    M130_REQUIRE(scope && *scope == 2);
    M130_REQUIRE(deref && *deref == 0);
    M130_REQUIRE(szl && *szl == 0);
    M130_REQUIRE(tlim && *tlim == 0);
    M130_REQUIRE(tonly && !*tonly);

    // Filter is AND (context 0, constructed) — verify its first byte.
    std::uint8_t ident = 0;
    const std::uint8_t* v = nullptr;
    std::size_t len = 0;
    M130_REQUIRE(sr->next(ident, v, len));
    M130_REQUIRE(ident == ber::identifier(ber::Class::ContextSpecific, true, 0));
    return 0;
}

int unbindRequestIsNull()
{
    auto blob = encodeUnbindRequest(99);
    // SEQUENCE { INTEGER 99, [APPLICATION 2] NULL }
    // [APPLICATION 2] is 0x42.
    ber::Decoder top(blob);
    auto env = top.readConstructed(ber::tag::kSequence);
    M130_REQUIRE(env.has_value());
    auto id = env->readInt();
    M130_REQUIRE(id && *id == 99);
    std::uint8_t ident = 0;
    const std::uint8_t* v = nullptr;
    std::size_t len = 0;
    M130_REQUIRE(env->next(ident, v, len));
    M130_REQUIRE(ident == 0x42);
    M130_REQUIRE(len == 0);
    return 0;
}

int filterEncodingPresentAndNot()
{
    auto filt = LdapFilter::parse("(!(objectClass=*))");
    M130_REQUIRE(filt.has_value());
    auto blob = encodeFilter(*filt);
    // NOT is [2] constructed, inner is present ([7] primitive) with value
    // "objectClass".
    ber::Decoder top(blob);
    auto notCtx = top.readConstructed(
        ber::identifier(ber::Class::ContextSpecific, true, 2));
    M130_REQUIRE(notCtx.has_value());
    std::uint8_t ident = 0;
    const std::uint8_t* v = nullptr;
    std::size_t len = 0;
    M130_REQUIRE(notCtx->next(ident, v, len));
    M130_REQUIRE(ident == ber::identifier(ber::Class::ContextSpecific, false, 7));
    M130_REQUIRE(std::string(reinterpret_cast<const char*>(v), len) == "objectClass");
    return 0;
}

int run()
{
    M130_RUN(encodesBindRequestStructure);
    M130_RUN(decodesBindResponseSuccess);
    M130_RUN(decodesBindResponseInvalidCredentials);
    M130_RUN(decodesSearchResultEntry);
    M130_RUN(encodesSearchRequestWithFilter);
    M130_RUN(unbindRequestIsNull);
    M130_RUN(filterEncodingPresentAndNot);
    return 0;
}

M130_TEST_MAIN()

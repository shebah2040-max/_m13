#include "access/Ber.h"
#include "test_support.h"

#include <cstring>
#include <string>

using namespace m130::access::ber;

int encodesAndDecodesBool()
{
    Encoder e;
    e.writeBool(true);
    e.writeBool(false);
    Decoder d(e.bytes());
    auto a = d.readBool();
    auto b = d.readBool();
    M130_REQUIRE(a.has_value() && *a == true);
    M130_REQUIRE(b.has_value() && *b == false);
    M130_REQUIRE(d.atEnd());
    return 0;
}

int encodesAndDecodesIntegers()
{
    const std::int64_t samples[] = {0, 1, 127, 128, 255, 256, -1, -128, -129, 1 << 16, -(1 << 20)};
    for (auto v : samples) {
        Encoder e;
        e.writeInt(v);
        Decoder d(e.bytes());
        auto out = d.readInt();
        M130_REQUIRE(out.has_value());
        M130_REQUIRE(*out == v);
        M130_REQUIRE(d.atEnd());
    }
    return 0;
}

int encodesAndDecodesEnumerated()
{
    Encoder e;
    e.writeEnum(42);
    Decoder d(e.bytes());
    auto out = d.readEnum();
    M130_REQUIRE(out.has_value() && *out == 42);
    return 0;
}

int encodesAndDecodesOctetString()
{
    Encoder e;
    e.writeOctetString("hello-ldap");
    Decoder d(e.bytes());
    auto s = d.readOctetString();
    M130_REQUIRE(s.has_value());
    M130_REQUIRE(*s == "hello-ldap");
    return 0;
}

int encodesAndDecodesNull()
{
    Encoder e;
    e.writeNull();
    Decoder d(e.bytes());
    M130_REQUIRE(d.readNull());
    M130_REQUIRE(d.atEnd());
    return 0;
}

int sequenceRoundTrip()
{
    Encoder e;
    const std::size_t outer = e.startConstructed(tag::kSequence);
    e.writeInt(7);
    e.writeOctetString("alice");
    e.writeBool(true);
    e.closeConstructed(outer);

    Decoder top(e.bytes());
    auto seq = top.readConstructed(tag::kSequence);
    M130_REQUIRE(seq.has_value());
    auto a = seq->readInt();
    auto b = seq->readOctetString();
    auto c = seq->readBool();
    M130_REQUIRE(a && *a == 7);
    M130_REQUIRE(b && *b == "alice");
    M130_REQUIRE(c && *c);
    M130_REQUIRE(seq->atEnd());
    M130_REQUIRE(top.atEnd());
    return 0;
}

int longFormLengthRoundTrip()
{
    std::string big(600, 'x');
    Encoder e;
    e.writeOctetString(big);
    Decoder d(e.bytes());
    auto out = d.readOctetString();
    M130_REQUIRE(out.has_value());
    M130_REQUIRE(*out == big);
    return 0;
}

int rejectsMalformedLength()
{
    std::vector<std::uint8_t> bad = {tag::kOctetString, 0x82, 0x00}; // length prefix missing byte
    Decoder d(bad);
    auto out = d.readOctetString();
    M130_REQUIRE(!out.has_value());
    return 0;
}

int applicationTagRoundTrip()
{
    const std::uint8_t bindReqIdent = identifier(Class::Application, true, 0);
    Encoder e;
    const std::size_t out_idx = e.startConstructed(bindReqIdent);
    e.writeInt(3); // LDAP v3
    e.writeOctetString("cn=admin,dc=m130,dc=local");
    e.writeRaw(identifier(Class::ContextSpecific, false, 0), std::string_view{"pw"});
    e.closeConstructed(out_idx);

    Decoder top(e.bytes());
    auto inner = top.readConstructed(bindReqIdent);
    M130_REQUIRE(inner.has_value());
    auto ver = inner->readInt();
    auto dn  = inner->readOctetString();
    std::uint8_t ident = 0;
    const std::uint8_t* value = nullptr;
    std::size_t len = 0;
    M130_REQUIRE(inner->next(ident, value, len));
    M130_REQUIRE(ver && *ver == 3);
    M130_REQUIRE(dn && *dn == "cn=admin,dc=m130,dc=local");
    M130_REQUIRE(ident == identifier(Class::ContextSpecific, false, 0));
    M130_REQUIRE(len == 2);
    M130_REQUIRE(std::memcmp(value, "pw", 2) == 0);
    return 0;
}

int run()
{
    M130_RUN(encodesAndDecodesBool);
    M130_RUN(encodesAndDecodesIntegers);
    M130_RUN(encodesAndDecodesEnumerated);
    M130_RUN(encodesAndDecodesOctetString);
    M130_RUN(encodesAndDecodesNull);
    M130_RUN(sequenceRoundTrip);
    M130_RUN(longFormLengthRoundTrip);
    M130_RUN(rejectsMalformedLength);
    M130_RUN(applicationTagRoundTrip);
    return 0;
}

M130_TEST_MAIN()

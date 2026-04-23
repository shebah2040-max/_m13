#include "access/LdapFilter.h"
#include "test_support.h"

using m130::access::LdapEntry;
using m130::access::LdapFilter;

namespace {
LdapEntry aliceEntry()
{
    LdapEntry e;
    e.add("uid",        "alice");
    e.add("cn",         "Alice Aviator");
    e.add("memberOf",   "cn=operators,ou=Groups,dc=m130,dc=local");
    e.add("memberOf",   "cn=observers,ou=Groups,dc=m130,dc=local");
    e.add("objectClass","inetOrgPerson");
    return e;
}
} // namespace

int equalsLeaf()
{
    const auto f = LdapFilter::parse("(uid=alice)");
    M130_REQUIRE(f.has_value());
    M130_REQUIRE(f->matches(aliceEntry()));
    const auto fbad = LdapFilter::parse("(uid=bob)");
    M130_REQUIRE(fbad.has_value());
    M130_REQUIRE(!fbad->matches(aliceEntry()));
    return 0;
}

int presenceLeaf()
{
    const auto f = LdapFilter::parse("(memberOf=*)");
    M130_REQUIRE(f.has_value());
    M130_REQUIRE(f->matches(aliceEntry()));
    const auto fbad = LdapFilter::parse("(telephone=*)");
    M130_REQUIRE(fbad.has_value());
    M130_REQUIRE(!fbad->matches(aliceEntry()));
    return 0;
}

int andOrNot()
{
    const auto land = LdapFilter::parse("(&(uid=alice)(objectClass=inetOrgPerson))");
    M130_REQUIRE(land.has_value());
    M130_REQUIRE(land->matches(aliceEntry()));

    const auto lor = LdapFilter::parse("(|(uid=bob)(uid=alice))");
    M130_REQUIRE(lor.has_value());
    M130_REQUIRE(lor->matches(aliceEntry()));

    const auto lnot = LdapFilter::parse("(!(uid=bob))");
    M130_REQUIRE(lnot.has_value());
    M130_REQUIRE(lnot->matches(aliceEntry()));

    const auto combined = LdapFilter::parse(
        "(&(memberOf=cn=operators,ou=Groups,dc=m130,dc=local)(!(uid=bob)))");
    M130_REQUIRE(combined.has_value());
    M130_REQUIRE(combined->matches(aliceEntry()));
    return 0;
}

int rejectsUnsupported()
{
    M130_REQUIRE(!LdapFilter::parse("(x~=y)").has_value());
    M130_REQUIRE(!LdapFilter::parse("(x>=1)").has_value());
    M130_REQUIRE(!LdapFilter::parse("(x<=1)").has_value());
    M130_REQUIRE(!LdapFilter::parse("uid=alice").has_value());   // no parens
    M130_REQUIRE(!LdapFilter::parse("(uid=alice").has_value());  // unclosed
    return 0;
}

int run()
{
    M130_RUN(equalsLeaf);
    M130_RUN(presenceLeaf);
    M130_RUN(andOrNot);
    M130_RUN(rejectsUnsupported);
    return 0;
}

M130_TEST_MAIN()

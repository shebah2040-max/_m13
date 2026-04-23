#include "access/LdapDn.h"
#include "test_support.h"

#include <string>

using m130::access::LdapDn;

int parsesSimpleDn()
{
    const auto dn = LdapDn::parse("uid=alice,ou=People,dc=m130,dc=local");
    M130_REQUIRE(dn.has_value());
    M130_REQUIRE(dn->pairs().size() == 4);
    M130_REQUIRE(dn->find("uid") == "alice");
    M130_REQUIRE(dn->find("ou")  == "People");
    M130_REQUIRE(dn->find("UID") == "alice"); // case-insensitive
    return 0;
}

int normalisesCanonical()
{
    const auto dn = LdapDn::parse("UID=alice,OU=People,DC=M130");
    M130_REQUIRE(dn.has_value());
    M130_REQUIRE_EQ(dn->canonical(), std::string("uid=alice,ou=People,dc=M130"));
    return 0;
}

int handlesEscapedValue()
{
    // Escape of comma via backslash.
    const auto dn = LdapDn::parse("cn=Doe\\, John,ou=Users,dc=m130");
    M130_REQUIRE(dn.has_value());
    M130_REQUIRE_EQ(dn->find("cn"), std::string("Doe, John"));
    return 0;
}

int rejectsMalformed()
{
    M130_REQUIRE(!LdapDn::parse("=alice,ou=People").has_value());  // empty attr
    M130_REQUIRE(!LdapDn::parse(",ou=People").has_value());        // leading comma
    return 0;
}

int emptyDnIsRoot()
{
    const auto dn = LdapDn::parse("");
    M130_REQUIRE(dn.has_value());
    M130_REQUIRE(dn->empty());
    M130_REQUIRE(dn->canonical().empty());
    return 0;
}

int run()
{
    M130_RUN(parsesSimpleDn);
    M130_RUN(normalisesCanonical);
    M130_RUN(handlesEscapedValue);
    M130_RUN(rejectsMalformed);
    M130_RUN(emptyDnIsRoot);
    return 0;
}

M130_TEST_MAIN()

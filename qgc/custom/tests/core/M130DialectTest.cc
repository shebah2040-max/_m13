#include "protocol/M130Dialect.h"
#include "test_support.h"

using namespace m130::protocol;

int idRangeCheck()
{
    M130_REQUIRE(isM130Id(42000));
    M130_REQUIRE(isM130Id(42107));
    M130_REQUIRE(!isM130Id(41999));
    M130_REQUIRE(!isM130Id(42256));
    return 0;
}

int describeKnown()
{
    auto d = describe(msg_id::kGncState);
    M130_REQUIRE_EQ(d.id, msg_id::kGncState);
    M130_REQUIRE_EQ(d.inbound, true);
    M130_REQUIRE(d.rate_hz > 0.0f);
    return 0;
}

int describeFts()
{
    auto d = describe(msg_id::kCommandFts);
    M130_REQUIRE_EQ(d.inbound, false);
    return 0;
}

int unknownNameFormat()
{
    auto n = nameOf(99999);
    M130_REQUIRE(n.rfind("UNKNOWN_", 0) == 0);
    return 0;
}

int run()
{
    M130_RUN(idRangeCheck);
    M130_RUN(describeKnown);
    M130_RUN(describeFts);
    M130_RUN(unknownNameFormat);
    return 0;
}

M130_TEST_MAIN()

#include "protocol/ProtocolVersion.h"
#include "test_support.h"

using namespace m130::protocol;

int packsAndUnpacks()
{
    ProtocolVersion v{ 2, 3, 4 };
    M130_REQUIRE_EQ(v.packed(), 0x020304u);
    ProtocolVersion u = ProtocolVersion::unpack(0x030201);
    M130_REQUIRE_EQ(u.major, uint8_t(3));
    M130_REQUIRE_EQ(u.minor, uint8_t(2));
    M130_REQUIRE_EQ(u.patch, uint8_t(1));
    return 0;
}

int compatSameIsCompatible()
{
    CompatReport r = checkCompat(kSupported);
    M130_REQUIRE_EQ(r.compat, VersionCompat::Compatible);
    return 0;
}

int compatMajorMismatch()
{
    ProtocolVersion p = kSupported;
    p.major = static_cast<uint8_t>(p.major + 1);
    CompatReport r = checkCompat(p);
    M130_REQUIRE_EQ(r.compat, VersionCompat::MajorMismatch);
    return 0;
}

int compatPatchDelta()
{
    ProtocolVersion p = kSupported;
    p.patch = static_cast<uint8_t>(p.patch + 1);
    CompatReport r = checkCompat(p);
    M130_REQUIRE_EQ(r.compat, VersionCompat::PatchDelta);
    return 0;
}

int run()
{
    M130_RUN(packsAndUnpacks);
    M130_RUN(compatSameIsCompatible);
    M130_RUN(compatMajorMismatch);
    M130_RUN(compatPatchDelta);
    return 0;
}

M130_TEST_MAIN()

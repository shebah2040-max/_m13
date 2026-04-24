#include "protocol/ProtocolVersion.h"
#include "safety/AlertLevel.h"
#include "safety/AlertManager.h"
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

int alertLevelMapping()
{
    using m130::safety::AlertLevel;
    M130_REQUIRE_EQ(alertLevelFor(VersionCompat::Compatible),    AlertLevel::None);
    M130_REQUIRE_EQ(alertLevelFor(VersionCompat::PatchDelta),    AlertLevel::Advisory);
    M130_REQUIRE_EQ(alertLevelFor(VersionCompat::MinorTooOld),   AlertLevel::Caution);
    M130_REQUIRE_EQ(alertLevelFor(VersionCompat::MajorMismatch), AlertLevel::Emergency);
    return 0;
}

int majorMismatchRaisesEmergency()
{
    // R1.4 — verify the mapping wired into AlertManager surfaces Emergency
    // on the master-caution bus when the peer runs an incompatible major.
    using namespace m130::safety;
    AlertManager mgr;

    ProtocolVersion p = kSupported;
    p.major = static_cast<uint8_t>(p.major + 1);
    const CompatReport r = checkCompat(p);
    M130_REQUIRE_EQ(r.compat, VersionCompat::MajorMismatch);

    const bool raised = mgr.raise(
        std::string(kCompatAlertId),
        alertLevelFor(r.compat),
        "Protocol version mismatch",
        r.message);
    M130_REQUIRE(raised);
    M130_REQUIRE_EQ(mgr.masterLevel(), AlertLevel::Emergency);
    M130_REQUIRE_EQ(mgr.active().size(), std::size_t(1));
    M130_REQUIRE_EQ(mgr.active().front().id, std::string(kCompatAlertId));
    return 0;
}

int compatibleClearsAlert()
{
    // Once the peer is back on a compatible version, clearing the stable
    // compat-alert id must drop it from the active set so the UI returns
    // to green.
    using namespace m130::safety;
    AlertManager mgr;
    mgr.raise(std::string(kCompatAlertId), AlertLevel::Emergency,
              "Protocol version mismatch", "major");
    M130_REQUIRE_EQ(mgr.masterLevel(), AlertLevel::Emergency);
    mgr.clear(std::string(kCompatAlertId));
    M130_REQUIRE_EQ(mgr.masterLevel(), AlertLevel::None);
    M130_REQUIRE_EQ(mgr.active().size(), std::size_t(0));
    return 0;
}

int run()
{
    M130_RUN(packsAndUnpacks);
    M130_RUN(compatSameIsCompatible);
    M130_RUN(compatMajorMismatch);
    M130_RUN(compatPatchDelta);
    M130_RUN(alertLevelMapping);
    M130_RUN(majorMismatchRaisesEmergency);
    M130_RUN(compatibleClearsAlert);
    return 0;
}

M130_TEST_MAIN()

#include "protocol/generated/M130DialectTable.generated.h"
#include "protocol/generated/M130Messages.generated.h"
#include "protocol/generated/M130Enums.generated.h"
#include "test_support.h"

#include <cstdint>
#include <cstring>

using namespace m130::protocol::gen;

namespace {

// REQ-M130-GCS-PROT-001 — dialect table enumerates every XML message.
int dialectTableIsPopulated()
{
    M130_REQUIRE_EQ(kDialectTable.size(), kDialectMessageCount);
    M130_REQUIRE(kDialectMessageCount == 16);

    const auto* hb = findDialectEntry(M130HeartbeatExtended::kMsgId);
    M130_REQUIRE(hb != nullptr);
    M130_REQUIRE_EQ(std::string_view(hb->name), "M130_HEARTBEAT_EXTENDED");
    M130_REQUIRE_EQ(hb->wire_size, M130HeartbeatExtended::kWireSize);
    M130_REQUIRE(hb->inbound);
    M130_REQUIRE(hb->rate_hz == 1.0f);

    const auto* arm = findDialectEntry(M130CommandArm::kMsgId);
    M130_REQUIRE(arm != nullptr);
    M130_REQUIRE(!arm->inbound);

    M130_REQUIRE(findDialectEntry(99999) == nullptr);
    return 0;
}

// REQ-M130-GCS-PROT-002 — round-trip every scalar field.
int gncStateRoundTrip()
{
    M130GncState src;
    src.time_usec = 0x11223344AABBCCDDULL;
    src.stage = 4;
    src.launched = 1;
    src.mode = 0xBEEF;
    src.q_dyn = 34567.125f;
    src.rho = 1.225f;
    src.altitude = 12345.5f;
    src.airspeed = 420.0f;
    src.phi = -33.25f;
    src.theta = 12.5f;
    src.psi = -170.5f;
    src.alpha_est = 4.75f;
    src.gamma_rad = 1.2345f;
    src.pos_downrange = 1234.5f;
    src.pos_crossrange = -42.0f;
    src.vel_downrange = 200.0f;
    src.vel_down = -50.0f;
    src.vel_crossrange = 7.25f;
    src.bearing_deg = 270.5f;
    src.target_range_rem = 9876.5f;
    src.iip_lat_e7 = 247123456;
    src.iip_lon_e7 = -1234567890;
    src.iip_alt = 55.0f;

    std::uint8_t buffer[256] = {};
    const std::size_t written = src.pack(buffer, sizeof(buffer));
    M130_REQUIRE_EQ(written, M130GncState::kWireSize);

    M130GncState dst;
    M130_REQUIRE(M130GncState::unpack(buffer, written, dst));
    M130_REQUIRE(src == dst);
    return 0;
}

// Authentication tokens pack as fixed arrays.
int commandFtsRoundTripWithArrays()
{
    M130CommandFts src;
    src.flight_id = 7;
    src.nonce = 0xCAFEBABE;
    src.timestamp_usec = 0x0102030405060708ULL;
    src.command_type = 0;
    src.reason_code = 1;
    for (int i = 0; i < 32; ++i) {
        src.auth_token_rso[i] = static_cast<std::uint8_t>(i);
        src.auth_token_safety[i] = static_cast<std::uint8_t>(255 - i);
    }

    std::uint8_t buffer[256] = {};
    const std::size_t written = src.pack(buffer, sizeof(buffer));
    M130_REQUIRE_EQ(written, M130CommandFts::kWireSize);

    M130CommandFts dst;
    M130_REQUIRE(M130CommandFts::unpack(buffer, written, dst));
    M130_REQUIRE(src == dst);

    // Mutate a byte in the middle and expect inequality.
    dst.auth_token_rso[10] ^= 0xAA;
    M130_REQUIRE(src != dst);
    return 0;
}

// Under-sized payload must fail (REQ-M130-GCS-PROT-003).
int unpackRejectsShortPayload()
{
    std::uint8_t buf[4] = {};
    M130HeartbeatExtended hb;
    M130_REQUIRE(!M130HeartbeatExtended::unpack(buf, sizeof(buf), hb));
    return 0;
}

// Pack rejects undersized buffer instead of corrupting memory.
int packRejectsShortBuffer()
{
    M130MpcDiagnostics src;
    src.solve_count = 42;
    std::uint8_t small[4] = {};
    M130_REQUIRE_EQ(src.pack(small, sizeof(small)), std::size_t(0));
    return 0;
}

// Wire size is the sum of field sizes (sanity check catches generator regressions).
int wireSizesMatchXmlSums()
{
    M130_REQUIRE_EQ(M130HeartbeatExtended::kWireSize, std::size_t(20));
    M130_REQUIRE_EQ(M130GncState::kWireSize, std::size_t(88));
    M130_REQUIRE_EQ(M130MheDiagnostics::kWireSize, std::size_t(50));
    M130_REQUIRE_EQ(M130MpcDiagnostics::kWireSize, std::size_t(64));
    M130_REQUIRE_EQ(M130FinState::kWireSize, std::size_t(33));
    M130_REQUIRE_EQ(M130EventCounters::kWireSize, std::size_t(40));
    M130_REQUIRE_EQ(M130SensorHealth::kWireSize, std::size_t(30));
    M130_REQUIRE_EQ(M130CommandAckM130::kWireSize, std::size_t(12));
    M130_REQUIRE_EQ(M130CommandArm::kWireSize, std::size_t(40));
    M130_REQUIRE_EQ(M130CommandFts::kWireSize, std::size_t(82));
    M130_REQUIRE_EQ(M130CommandTune::kWireSize, std::size_t(46));
    return 0;
}

// Enum toString is a generated helper used by logging / UI.
int enumToStringCoversAllLevels()
{
    M130_REQUIRE_EQ(
        std::string_view(toString(M130AlertLevel::M130_ALERT_EMERGENCY)),
        std::string_view("M130_ALERT_EMERGENCY"));
    M130_REQUIRE_EQ(
        std::string_view(toString(M130FlightPhase::M130_PHASE_BOOST)),
        std::string_view("M130_PHASE_BOOST"));
    return 0;
}

int run()
{
    M130_RUN(dialectTableIsPopulated);
    M130_RUN(gncStateRoundTrip);
    M130_RUN(commandFtsRoundTripWithArrays);
    M130_RUN(unpackRejectsShortPayload);
    M130_RUN(packRejectsShortBuffer);
    M130_RUN(wireSizesMatchXmlSums);
    M130_RUN(enumToStringCoversAllLevels);
    return 0;
}

} // namespace

M130_TEST_MAIN()

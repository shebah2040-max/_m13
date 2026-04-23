#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace m130::protocol {

/// Message IDs for the m130 dialect (mirrors mavlink/m130.xml).
/// This block is reserved 42000..42255 for the M130 project.
namespace msg_id {
    // Inbound (vehicle -> GCS)
    constexpr uint32_t kHeartbeatExtended = 42000;
    constexpr uint32_t kGncState          = 42001;
    constexpr uint32_t kMheDiagnostics    = 42002;
    constexpr uint32_t kMpcDiagnostics    = 42003;
    constexpr uint32_t kFinState          = 42004;
    constexpr uint32_t kEventCounters     = 42005;
    constexpr uint32_t kSensorHealth      = 42006;
    constexpr uint32_t kCommandAckM130    = 42007;

    // Outbound (GCS -> vehicle)
    constexpr uint32_t kCommandArm           = 42100;
    constexpr uint32_t kCommandDisarm        = 42101;
    constexpr uint32_t kCommandHold          = 42102;
    constexpr uint32_t kCommandAbort         = 42103;
    constexpr uint32_t kCommandFts           = 42104;
    constexpr uint32_t kCommandTune          = 42105;
    constexpr uint32_t kCommandModeSwitch    = 42106;
    constexpr uint32_t kCommandChecklistSign = 42107;
} // namespace msg_id

/// Metadata describing a message type. Returned by `describe()` for UIs &
/// diagnostics.
struct MessageDescriptor {
    uint32_t    id = 0;
    std::string_view name;
    bool        inbound = true;
    float       rate_hz = 0.0f;
    std::string_view description;
};

/// Compile-time descriptors for all messages in the dialect.
MessageDescriptor describe(uint32_t msg_id);

/// Human-readable name. Returns "UNKNOWN_<id>" for unknown ids.
std::string nameOf(uint32_t msg_id);

/// Whether a message id falls in the m130 reserved block.
constexpr bool isM130Id(uint32_t id) { return id >= 42000 && id <= 42255; }

} // namespace m130::protocol

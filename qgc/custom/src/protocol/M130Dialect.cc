#include "M130Dialect.h"

#include <array>

namespace m130::protocol {

namespace {

constexpr std::array<MessageDescriptor, 16> kDescriptors = {{
    { msg_id::kHeartbeatExtended,   "M130_HEARTBEAT_EXTENDED",   true,  1.0f,  "Extended heartbeat with protocol version" },
    { msg_id::kGncState,            "M130_GNC_STATE",            true,  20.0f, "Primary GNC state" },
    { msg_id::kMheDiagnostics,      "M130_MHE_DIAGNOSTICS",      true,  10.0f, "MHE diagnostics" },
    { msg_id::kMpcDiagnostics,      "M130_MPC_DIAGNOSTICS",      true,  10.0f, "MPC diagnostics" },
    { msg_id::kFinState,            "M130_FIN_STATE",            true,  20.0f, "Fin actuator state" },
    { msg_id::kEventCounters,       "M130_EVENT_COUNTERS",       true,  1.0f,  "Aggregated event counters" },
    { msg_id::kSensorHealth,        "M130_SENSOR_HEALTH",        true,  1.0f,  "Sensor subsystem health" },
    { msg_id::kCommandAckM130,      "M130_COMMAND_ACK_M130",     true,  0.0f,  "Custom ACK" },
    { msg_id::kCommandArm,          "M130_COMMAND_ARM",          false, 0.0f,  "Arm the vehicle" },
    { msg_id::kCommandDisarm,       "M130_COMMAND_DISARM",       false, 0.0f,  "Disarm" },
    { msg_id::kCommandHold,         "M130_COMMAND_HOLD",         false, 0.0f,  "Hold current phase" },
    { msg_id::kCommandAbort,        "M130_COMMAND_ABORT",        false, 0.0f,  "Graceful abort" },
    { msg_id::kCommandFts,          "M130_COMMAND_FTS",          false, 0.0f,  "Flight Termination (dual-auth)" },
    { msg_id::kCommandTune,         "M130_COMMAND_TUNE",         false, 0.0f,  "In-flight tune parameter" },
    { msg_id::kCommandModeSwitch,   "M130_COMMAND_MODE_SWITCH",  false, 0.0f,  "Switch flight mode" },
    { msg_id::kCommandChecklistSign,"M130_COMMAND_CHECKLIST_SIGN",false,0.0f,  "Sign a pre-launch checklist item" },
}};

} // namespace

MessageDescriptor describe(uint32_t id)
{
    for (const auto& d : kDescriptors) {
        if (d.id == id) return d;
    }
    MessageDescriptor unknown;
    unknown.id = id;
    unknown.name = std::string_view{};
    unknown.description = "unknown";
    return unknown;
}

std::string nameOf(uint32_t id)
{
    for (const auto& d : kDescriptors) {
        if (d.id == id) return std::string(d.name);
    }
    return std::string("UNKNOWN_") + std::to_string(id);
}

} // namespace m130::protocol

// SPDX-License-Identifier: GPL-3.0-or-later
// AUTO-GENERATED from qgc/custom/mavlink/m130.xml by tools/generate-dialect.py.
// DO NOT EDIT. Re-run the generator when the dialect changes.

#include "M130DialectTable.generated.h"
#include "M130Messages.generated.h"

namespace m130::protocol::gen {

const std::array<DialectEntry, kDialectMessageCount> kDialectTable{{
    {42000, "M130_HEARTBEAT_EXTENDED", 20, true, 1.0f},
    {42001, "M130_GNC_STATE", 88, true, 20.0f},
    {42002, "M130_MHE_DIAGNOSTICS", 50, true, 10.0f},
    {42003, "M130_MPC_DIAGNOSTICS", 64, true, 10.0f},
    {42004, "M130_FIN_STATE", 33, true, 20.0f},
    {42005, "M130_EVENT_COUNTERS", 40, true, 1.0f},
    {42006, "M130_SENSOR_HEALTH", 30, true, 1.0f},
    {42007, "M130_COMMAND_ACK_M130", 12, true, 0.0f},
    {42100, "M130_COMMAND_ARM", 40, false, 0.0f},
    {42101, "M130_COMMAND_DISARM", 40, false, 0.0f},
    {42102, "M130_COMMAND_HOLD", 40, false, 0.0f},
    {42103, "M130_COMMAND_ABORT", 41, false, 0.0f},
    {42104, "M130_COMMAND_FTS", 82, false, 0.0f},
    {42105, "M130_COMMAND_TUNE", 46, false, 0.0f},
    {42106, "M130_COMMAND_MODE_SWITCH", 42, false, 0.0f},
    {42107, "M130_COMMAND_CHECKLIST_SIGN", 41, false, 0.0f},
}};

const DialectEntry* findDialectEntry(std::uint32_t msg_id) noexcept {
    for (const auto& e : kDialectTable) {
        if (e.msg_id == msg_id) return &e;
    }
    return nullptr;
}

} // namespace m130::protocol::gen

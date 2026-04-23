// SPDX-License-Identifier: GPL-3.0-or-later
// AUTO-GENERATED from qgc/custom/mavlink/m130.xml by tools/generate-dialect.py.
// DO NOT EDIT. Re-run the generator when the dialect changes.

#pragma once

#include <cstdint>

namespace m130::protocol::gen {

/// Mission state machine phases — matches src/safety/MissionStateMachine.h
enum class M130FlightPhase : std::uint32_t {
    M130_PHASE_UNKNOWN = 0,
    M130_PHASE_IDLE = 1,
    M130_PHASE_PRELAUNCH = 2,
    M130_PHASE_ARMED = 3,
    M130_PHASE_BOOST = 4,
    M130_PHASE_CRUISE = 5,
    M130_PHASE_TERMINAL = 6,
    M130_PHASE_LANDED = 7,
    M130_PHASE_ABORT = 8,
};

constexpr const char* toString(M130FlightPhase v) noexcept {
    switch (v) {
        case M130FlightPhase::M130_PHASE_UNKNOWN: return "M130_PHASE_UNKNOWN";
        case M130FlightPhase::M130_PHASE_IDLE: return "M130_PHASE_IDLE";
        case M130FlightPhase::M130_PHASE_PRELAUNCH: return "M130_PHASE_PRELAUNCH";
        case M130FlightPhase::M130_PHASE_ARMED: return "M130_PHASE_ARMED";
        case M130FlightPhase::M130_PHASE_BOOST: return "M130_PHASE_BOOST";
        case M130FlightPhase::M130_PHASE_CRUISE: return "M130_PHASE_CRUISE";
        case M130FlightPhase::M130_PHASE_TERMINAL: return "M130_PHASE_TERMINAL";
        case M130FlightPhase::M130_PHASE_LANDED: return "M130_PHASE_LANDED";
        case M130FlightPhase::M130_PHASE_ABORT: return "M130_PHASE_ABORT";
    }
    return "?";
}

/// ARINC 661 alert levels — matches src/safety/AlertManager.h
enum class M130AlertLevel : std::uint32_t {
    M130_ALERT_NONE = 0,
    M130_ALERT_ADVISORY = 1,
    M130_ALERT_CAUTION = 2,
    M130_ALERT_WARNING = 3,
    M130_ALERT_EMERGENCY = 4,
};

constexpr const char* toString(M130AlertLevel v) noexcept {
    switch (v) {
        case M130AlertLevel::M130_ALERT_NONE: return "M130_ALERT_NONE";
        case M130AlertLevel::M130_ALERT_ADVISORY: return "M130_ALERT_ADVISORY";
        case M130AlertLevel::M130_ALERT_CAUTION: return "M130_ALERT_CAUTION";
        case M130AlertLevel::M130_ALERT_WARNING: return "M130_ALERT_WARNING";
        case M130AlertLevel::M130_ALERT_EMERGENCY: return "M130_ALERT_EMERGENCY";
    }
    return "?";
}

/// Bitmask in M130_HEARTBEAT_EXTENDED.system_state
enum class M130SystemStateFlags : std::uint32_t {
    M130_STATE_ARMED = 1,
    M130_STATE_LAUNCHED = 2,
    M130_STATE_SAFETY_OK = 4,
    M130_STATE_SOLVER_OK = 8,
    M130_STATE_SENSORS_OK = 16,
    M130_STATE_LINK_OK = 32,
    M130_STATE_FTS_ARMED = 64,
    M130_STATE_FTS_FIRED = 128,
    M130_STATE_ABORT_REQUESTED = 256,
    M130_STATE_HOLD_ACTIVE = 512,
    M130_STATE_DEGRADED = 1024,
};

constexpr const char* toString(M130SystemStateFlags v) noexcept {
    switch (v) {
        case M130SystemStateFlags::M130_STATE_ARMED: return "M130_STATE_ARMED";
        case M130SystemStateFlags::M130_STATE_LAUNCHED: return "M130_STATE_LAUNCHED";
        case M130SystemStateFlags::M130_STATE_SAFETY_OK: return "M130_STATE_SAFETY_OK";
        case M130SystemStateFlags::M130_STATE_SOLVER_OK: return "M130_STATE_SOLVER_OK";
        case M130SystemStateFlags::M130_STATE_SENSORS_OK: return "M130_STATE_SENSORS_OK";
        case M130SystemStateFlags::M130_STATE_LINK_OK: return "M130_STATE_LINK_OK";
        case M130SystemStateFlags::M130_STATE_FTS_ARMED: return "M130_STATE_FTS_ARMED";
        case M130SystemStateFlags::M130_STATE_FTS_FIRED: return "M130_STATE_FTS_FIRED";
        case M130SystemStateFlags::M130_STATE_ABORT_REQUESTED: return "M130_STATE_ABORT_REQUESTED";
        case M130SystemStateFlags::M130_STATE_HOLD_ACTIVE: return "M130_STATE_HOLD_ACTIVE";
        case M130SystemStateFlags::M130_STATE_DEGRADED: return "M130_STATE_DEGRADED";
    }
    return "?";
}

enum class M130SolverStatus : std::uint32_t {
    M130_SOLVER_OK = 0,
    M130_SOLVER_FAIL = 1,
    M130_SOLVER_MAX_ITER = 2,
    M130_SOLVER_NAN = 3,
    M130_SOLVER_INFEASIBLE = 4,
};

constexpr const char* toString(M130SolverStatus v) noexcept {
    switch (v) {
        case M130SolverStatus::M130_SOLVER_OK: return "M130_SOLVER_OK";
        case M130SolverStatus::M130_SOLVER_FAIL: return "M130_SOLVER_FAIL";
        case M130SolverStatus::M130_SOLVER_MAX_ITER: return "M130_SOLVER_MAX_ITER";
        case M130SolverStatus::M130_SOLVER_NAN: return "M130_SOLVER_NAN";
        case M130SolverStatus::M130_SOLVER_INFEASIBLE: return "M130_SOLVER_INFEASIBLE";
    }
    return "?";
}

enum class M130FtsReason : std::uint32_t {
    M130_FTS_REASON_MANUAL_RSO = 0,
    M130_FTS_REASON_ENVELOPE_VIOLATION = 1,
    M130_FTS_REASON_LOSS_OF_CONTROL = 2,
    M130_FTS_REASON_LINK_LOSS = 3,
    M130_FTS_REASON_IIP_OUT_OF_CORRIDOR = 4,
    M130_FTS_REASON_BATTERY_CRITICAL = 5,
};

constexpr const char* toString(M130FtsReason v) noexcept {
    switch (v) {
        case M130FtsReason::M130_FTS_REASON_MANUAL_RSO: return "M130_FTS_REASON_MANUAL_RSO";
        case M130FtsReason::M130_FTS_REASON_ENVELOPE_VIOLATION: return "M130_FTS_REASON_ENVELOPE_VIOLATION";
        case M130FtsReason::M130_FTS_REASON_LOSS_OF_CONTROL: return "M130_FTS_REASON_LOSS_OF_CONTROL";
        case M130FtsReason::M130_FTS_REASON_LINK_LOSS: return "M130_FTS_REASON_LINK_LOSS";
        case M130FtsReason::M130_FTS_REASON_IIP_OUT_OF_CORRIDOR: return "M130_FTS_REASON_IIP_OUT_OF_CORRIDOR";
        case M130FtsReason::M130_FTS_REASON_BATTERY_CRITICAL: return "M130_FTS_REASON_BATTERY_CRITICAL";
    }
    return "?";
}

enum class M130CmdAckResult : std::uint32_t {
    M130_ACK_OK = 0,
    M130_ACK_REJECTED_AUTH = 1,
    M130_ACK_REJECTED_STATE = 2,
    M130_ACK_REJECTED_RANGE = 3,
    M130_ACK_REJECTED_VERSION = 4,
    M130_ACK_TIMEOUT = 5,
    M130_ACK_FTS_ARMED = 6,
    M130_ACK_FTS_FIRED = 7,
};

constexpr const char* toString(M130CmdAckResult v) noexcept {
    switch (v) {
        case M130CmdAckResult::M130_ACK_OK: return "M130_ACK_OK";
        case M130CmdAckResult::M130_ACK_REJECTED_AUTH: return "M130_ACK_REJECTED_AUTH";
        case M130CmdAckResult::M130_ACK_REJECTED_STATE: return "M130_ACK_REJECTED_STATE";
        case M130CmdAckResult::M130_ACK_REJECTED_RANGE: return "M130_ACK_REJECTED_RANGE";
        case M130CmdAckResult::M130_ACK_REJECTED_VERSION: return "M130_ACK_REJECTED_VERSION";
        case M130CmdAckResult::M130_ACK_TIMEOUT: return "M130_ACK_TIMEOUT";
        case M130CmdAckResult::M130_ACK_FTS_ARMED: return "M130_ACK_FTS_ARMED";
        case M130CmdAckResult::M130_ACK_FTS_FIRED: return "M130_ACK_FTS_FIRED";
    }
    return "?";
}

} // namespace m130::protocol::gen

// SPDX-License-Identifier: GPL-3.0-or-later
// AUTO-GENERATED from qgc/custom/mavlink/m130.xml by tools/generate-dialect.py.
// DO NOT EDIT. Re-run the generator when the dialect changes.

#pragma once

#include "M130Enums.generated.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace m130::protocol::gen {

/// msgid 42000 — Extended heartbeat from rocket_mpc @ 1 Hz.
struct M130HeartbeatExtended {
    static constexpr std::uint32_t kMsgId = 42000;
    static constexpr std::size_t kWireSize = 20;
    static constexpr const char* kName = "M130_HEARTBEAT_EXTENDED";

    std::uint32_t protocol_version = {};  ///< Major<<16 | Minor<<8 | Patch
    std::uint32_t system_state = {};  ///< Bitmask M130_SYSTEM_STATE_FLAGS
    std::uint32_t uptime_s = {};  ///< Seconds since boot
    std::uint32_t flight_id = {};  ///< Incremented on each ARM
    std::uint16_t flags = {};  ///< Reserved / custom flags
    std::uint8_t flight_phase = {};  ///< enum=M130_FLIGHT_PHASE | Current mission phase
    std::uint8_t alert_level = {};  ///< enum=M130_ALERT_LEVEL | Highest active alert

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130HeartbeatExtended& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130HeartbeatExtended& rhs) const noexcept;
    bool operator!=(const M130HeartbeatExtended& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42001 — Primary GNC state @ 20 Hz.
struct M130GncState {
    static constexpr std::uint32_t kMsgId = 42001;
    static constexpr std::size_t kWireSize = 88;
    static constexpr const char* kName = "M130_GNC_STATE";

    std::uint64_t time_usec = {};  ///< units=us | Boot time
    std::uint8_t stage = {};  ///< enum=M130_FLIGHT_PHASE
    std::uint8_t launched = {};
    std::uint16_t mode = {};
    float q_dyn = {};  ///< units=Pa
    float rho = {};  ///< units=kg/m^3
    float altitude = {};  ///< units=m | Above takeoff
    float airspeed = {};  ///< units=m/s
    float phi = {};  ///< units=deg
    float theta = {};  ///< units=deg
    float psi = {};  ///< units=deg
    float alpha_est = {};  ///< units=deg
    float gamma_rad = {};  ///< units=rad
    float pos_downrange = {};  ///< units=m
    float pos_crossrange = {};  ///< units=m
    float vel_downrange = {};  ///< units=m/s
    float vel_down = {};  ///< units=m/s
    float vel_crossrange = {};  ///< units=m/s
    float bearing_deg = {};  ///< units=deg
    float target_range_rem = {};  ///< units=m
    std::int32_t iip_lat_e7 = {};  ///< units=degE7 | Instantaneous Impact Point latitude
    std::int32_t iip_lon_e7 = {};  ///< units=degE7 | Instantaneous Impact Point longitude
    float iip_alt = {};  ///< units=m

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130GncState& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130GncState& rhs) const noexcept;
    bool operator!=(const M130GncState& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42002 — MHE estimator diagnostics @ 10 Hz.
struct M130MheDiagnostics {
    static constexpr std::uint32_t kMsgId = 42002;
    static constexpr std::size_t kWireSize = 50;
    static constexpr const char* kName = "M130_MHE_DIAGNOSTICS";

    std::uint64_t time_usec = {};  ///< units=us
    std::uint32_t solve_count = {};
    std::uint32_t fail_count = {};
    std::uint32_t solve_us = {};  ///< units=us
    float quality = {};  ///< units=ratio
    float blend_alpha = {};  ///< units=ratio
    float xval_gamma_err = {};  ///< units=deg
    float xval_chi_err = {};  ///< units=deg
    float xval_alt_err = {};  ///< units=m
    float xval_penalty = {};
    std::uint8_t valid = {};
    std::uint8_t status = {};  ///< enum=M130_SOLVER_STATUS
    std::uint32_t reset_count = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130MheDiagnostics& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130MheDiagnostics& rhs) const noexcept;
    bool operator!=(const M130MheDiagnostics& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42003 — MPC controller diagnostics @ 10 Hz.
struct M130MpcDiagnostics {
    static constexpr std::uint32_t kMsgId = 42003;
    static constexpr std::size_t kWireSize = 64;
    static constexpr const char* kName = "M130_MPC_DIAGNOSTICS";

    std::uint64_t time_usec = {};  ///< units=us
    std::uint32_t solve_count = {};
    std::uint32_t fail_count = {};
    std::uint32_t nan_skip_count = {};
    std::uint32_t solve_us = {};  ///< units=us
    std::uint16_t sqp_iter = {};
    std::uint16_t status = {};  ///< enum=M130_SOLVER_STATUS
    float cost_total = {};
    float cost_stage = {};
    float cost_terminal = {};
    float constraint_violation = {};
    float delta_roll_cmd = {};  ///< units=deg
    float delta_pitch_cmd = {};  ///< units=deg
    float delta_yaw_cmd = {};  ///< units=deg
    float pitch_accel_cmd = {};  ///< units=rad/s/s
    float yaw_accel_cmd = {};  ///< units=rad/s/s

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130MpcDiagnostics& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130MpcDiagnostics& rhs) const noexcept;
    bool operator!=(const M130MpcDiagnostics& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42004 — Fin actuator state @ 20 Hz.
struct M130FinState {
    static constexpr std::uint32_t kMsgId = 42004;
    static constexpr std::size_t kWireSize = 33;
    static constexpr const char* kName = "M130_FIN_STATE";

    std::uint64_t time_usec = {};  ///< units=us
    float fin1_deg = {};  ///< units=deg
    float fin2_deg = {};  ///< units=deg
    float fin3_deg = {};  ///< units=deg
    float fin4_deg = {};  ///< units=deg
    std::uint8_t servo_online_mask = {};
    std::uint32_t clamp_count = {};
    std::uint32_t offline_events = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130FinState& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130FinState& rhs) const noexcept;
    bool operator!=(const M130FinState& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42005 — Aggregated event counters @ 1 Hz.
struct M130EventCounters {
    static constexpr std::uint32_t kMsgId = 42005;
    static constexpr std::size_t kWireSize = 40;
    static constexpr const char* kName = "M130_EVENT_COUNTERS";

    std::uint64_t time_usec = {};  ///< units=us
    std::uint32_t mpc_fail_count = {};
    std::uint32_t mpc_nan_skip_count = {};
    std::uint32_t mhe_fail_count = {};
    std::uint32_t fin_clamp_count = {};
    std::uint32_t xval_reset_count = {};
    std::uint32_t servo_offline_events = {};
    std::uint32_t link_loss_events = {};
    std::uint32_t envelope_violations = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130EventCounters& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130EventCounters& rhs) const noexcept;
    bool operator!=(const M130EventCounters& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42006 — Sensor subsystem health @ 1 Hz.
struct M130SensorHealth {
    static constexpr std::uint32_t kMsgId = 42006;
    static constexpr std::size_t kWireSize = 30;
    static constexpr const char* kName = "M130_SENSOR_HEALTH";

    std::uint64_t time_usec = {};  ///< units=us
    std::uint8_t imu_health = {};
    std::uint8_t baro_health = {};
    std::uint8_t mag_health = {};
    std::uint8_t gps_fix_type = {};
    std::uint8_t gps_satellites = {};
    std::uint8_t gps_jamming_state = {};
    float imu_temp_c = {};  ///< units=degC
    float battery_v = {};  ///< units=V
    float battery_a = {};  ///< units=A
    float battery_pct = {};  ///< units=ratio

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130SensorHealth& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130SensorHealth& rhs) const noexcept;
    bool operator!=(const M130SensorHealth& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42007 — Custom ACK for M130 commands (id 42100+).
struct M130CommandAckM130 {
    static constexpr std::uint32_t kMsgId = 42007;
    static constexpr std::size_t kWireSize = 12;
    static constexpr const char* kName = "M130_COMMAND_ACK_M130";

    std::uint32_t command_id = {};  ///< mavlink msg id being acked
    std::uint32_t flight_id = {};
    std::uint8_t result = {};  ///< enum=M130_CMD_ACK_RESULT
    std::uint8_t reason_code = {};
    std::uint16_t progress_pct = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130CommandAckM130& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130CommandAckM130& rhs) const noexcept;
    bool operator!=(const M130CommandAckM130& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42100 — Arm the vehicle. Auth: Operator+.
struct M130CommandArm {
    static constexpr std::uint32_t kMsgId = 42100;
    static constexpr std::size_t kWireSize = 40;
    static constexpr const char* kName = "M130_COMMAND_ARM";

    std::uint32_t flight_id = {};
    std::uint32_t nonce = {};
    std::uint8_t auth_token[32] = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130CommandArm& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130CommandArm& rhs) const noexcept;
    bool operator!=(const M130CommandArm& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42101 — Disarm. Auth: Operator+.
struct M130CommandDisarm {
    static constexpr std::uint32_t kMsgId = 42101;
    static constexpr std::size_t kWireSize = 40;
    static constexpr const char* kName = "M130_COMMAND_DISARM";

    std::uint32_t flight_id = {};
    std::uint32_t nonce = {};
    std::uint8_t auth_token[32] = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130CommandDisarm& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130CommandDisarm& rhs) const noexcept;
    bool operator!=(const M130CommandDisarm& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42102 — Hold current phase. Auth: Safety Officer+.
struct M130CommandHold {
    static constexpr std::uint32_t kMsgId = 42102;
    static constexpr std::size_t kWireSize = 40;
    static constexpr const char* kName = "M130_COMMAND_HOLD";

    std::uint32_t flight_id = {};
    std::uint32_t nonce = {};
    std::uint8_t auth_token[32] = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130CommandHold& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130CommandHold& rhs) const noexcept;
    bool operator!=(const M130CommandHold& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42103 — Graceful mission abort. Auth: Safety Officer+.
struct M130CommandAbort {
    static constexpr std::uint32_t kMsgId = 42103;
    static constexpr std::size_t kWireSize = 41;
    static constexpr const char* kName = "M130_COMMAND_ABORT";

    std::uint32_t flight_id = {};
    std::uint32_t nonce = {};
    std::uint8_t reason_code = {};
    std::uint8_t auth_token[32] = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130CommandAbort& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130CommandAbort& rhs) const noexcept;
    bool operator!=(const M130CommandAbort& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42104 — Flight Termination. Auth: RSO AND Safety Officer (dual-auth).
struct M130CommandFts {
    static constexpr std::uint32_t kMsgId = 42104;
    static constexpr std::size_t kWireSize = 82;
    static constexpr const char* kName = "M130_COMMAND_FTS";

    std::uint32_t flight_id = {};
    std::uint32_t nonce = {};
    std::uint64_t timestamp_usec = {};
    std::uint8_t command_type = {};  ///< 0=full FTS, 1=engine cutoff only
    std::uint8_t reason_code = {};  ///< enum=M130_FTS_REASON
    std::uint8_t auth_token_rso[32] = {};
    std::uint8_t auth_token_safety[32] = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130CommandFts& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130CommandFts& rhs) const noexcept;
    bool operator!=(const M130CommandFts& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42105 — In-flight tune MPC/MHE parameters (bounded). Auth: Flight Director+.
struct M130CommandTune {
    static constexpr std::uint32_t kMsgId = 42105;
    static constexpr std::size_t kWireSize = 46;
    static constexpr const char* kName = "M130_COMMAND_TUNE";

    std::uint32_t flight_id = {};
    std::uint32_t nonce = {};
    std::uint16_t param_id = {};
    float new_value = {};
    std::uint8_t auth_token[32] = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130CommandTune& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130CommandTune& rhs) const noexcept;
    bool operator!=(const M130CommandTune& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42106 — Switch flight mode. Auth: Operator+.
struct M130CommandModeSwitch {
    static constexpr std::uint32_t kMsgId = 42106;
    static constexpr std::size_t kWireSize = 42;
    static constexpr const char* kName = "M130_COMMAND_MODE_SWITCH";

    std::uint32_t flight_id = {};
    std::uint32_t nonce = {};
    std::uint16_t new_mode = {};
    std::uint8_t auth_token[32] = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130CommandModeSwitch& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130CommandModeSwitch& rhs) const noexcept;
    bool operator!=(const M130CommandModeSwitch& rhs) const noexcept { return !(*this == rhs); }
};

/// msgid 42107 — Sign off a pre-launch checklist item. Auth: Operator+.
struct M130CommandChecklistSign {
    static constexpr std::uint32_t kMsgId = 42107;
    static constexpr std::size_t kWireSize = 41;
    static constexpr const char* kName = "M130_COMMAND_CHECKLIST_SIGN";

    std::uint32_t flight_id = {};
    std::uint32_t item_id = {};
    std::uint8_t result = {};  ///< 0=NO-GO, 1=GO, 2=HOLD
    std::uint8_t auth_token[32] = {};

    static bool unpack(const std::uint8_t* payload, std::size_t len, M130CommandChecklistSign& out) noexcept;
    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;
    bool operator==(const M130CommandChecklistSign& rhs) const noexcept;
    bool operator!=(const M130CommandChecklistSign& rhs) const noexcept { return !(*this == rhs); }
};

} // namespace m130::protocol::gen

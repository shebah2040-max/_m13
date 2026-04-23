// SPDX-License-Identifier: GPL-3.0-or-later
// AUTO-GENERATED from qgc/custom/mavlink/m130.xml by tools/generate-dialect.py.
// DO NOT EDIT. Re-run the generator when the dialect changes.

#include "M130Messages.generated.h"

#include <cstring>

namespace m130::protocol::gen {

bool M130HeartbeatExtended::unpack(const std::uint8_t* payload, std::size_t len, M130HeartbeatExtended& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.protocol_version, payload + off, 4); off += 4;
    std::memcpy(&out.system_state, payload + off, 4); off += 4;
    std::memcpy(&out.uptime_s, payload + off, 4); off += 4;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.flags, payload + off, 2); off += 2;
    std::memcpy(&out.flight_phase, payload + off, 1); off += 1;
    std::memcpy(&out.alert_level, payload + off, 1); off += 1;
    (void) off;
    return true;
}

std::size_t M130HeartbeatExtended::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &protocol_version, 4); off += 4;
    std::memcpy(payload + off, &system_state, 4); off += 4;
    std::memcpy(payload + off, &uptime_s, 4); off += 4;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &flags, 2); off += 2;
    std::memcpy(payload + off, &flight_phase, 1); off += 1;
    std::memcpy(payload + off, &alert_level, 1); off += 1;
    return off;
}

bool M130HeartbeatExtended::operator==(const M130HeartbeatExtended& rhs) const noexcept {
    return protocol_version == rhs.protocol_version
        && system_state == rhs.system_state
        && uptime_s == rhs.uptime_s
        && flight_id == rhs.flight_id
        && flags == rhs.flags
        && flight_phase == rhs.flight_phase
        && alert_level == rhs.alert_level;
}

bool M130GncState::unpack(const std::uint8_t* payload, std::size_t len, M130GncState& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.time_usec, payload + off, 8); off += 8;
    std::memcpy(&out.q_dyn, payload + off, 4); off += 4;
    std::memcpy(&out.rho, payload + off, 4); off += 4;
    std::memcpy(&out.altitude, payload + off, 4); off += 4;
    std::memcpy(&out.airspeed, payload + off, 4); off += 4;
    std::memcpy(&out.phi, payload + off, 4); off += 4;
    std::memcpy(&out.theta, payload + off, 4); off += 4;
    std::memcpy(&out.psi, payload + off, 4); off += 4;
    std::memcpy(&out.alpha_est, payload + off, 4); off += 4;
    std::memcpy(&out.gamma_rad, payload + off, 4); off += 4;
    std::memcpy(&out.pos_downrange, payload + off, 4); off += 4;
    std::memcpy(&out.pos_crossrange, payload + off, 4); off += 4;
    std::memcpy(&out.vel_downrange, payload + off, 4); off += 4;
    std::memcpy(&out.vel_down, payload + off, 4); off += 4;
    std::memcpy(&out.vel_crossrange, payload + off, 4); off += 4;
    std::memcpy(&out.bearing_deg, payload + off, 4); off += 4;
    std::memcpy(&out.target_range_rem, payload + off, 4); off += 4;
    std::memcpy(&out.iip_lat_e7, payload + off, 4); off += 4;
    std::memcpy(&out.iip_lon_e7, payload + off, 4); off += 4;
    std::memcpy(&out.iip_alt, payload + off, 4); off += 4;
    std::memcpy(&out.mode, payload + off, 2); off += 2;
    std::memcpy(&out.stage, payload + off, 1); off += 1;
    std::memcpy(&out.launched, payload + off, 1); off += 1;
    (void) off;
    return true;
}

std::size_t M130GncState::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &time_usec, 8); off += 8;
    std::memcpy(payload + off, &q_dyn, 4); off += 4;
    std::memcpy(payload + off, &rho, 4); off += 4;
    std::memcpy(payload + off, &altitude, 4); off += 4;
    std::memcpy(payload + off, &airspeed, 4); off += 4;
    std::memcpy(payload + off, &phi, 4); off += 4;
    std::memcpy(payload + off, &theta, 4); off += 4;
    std::memcpy(payload + off, &psi, 4); off += 4;
    std::memcpy(payload + off, &alpha_est, 4); off += 4;
    std::memcpy(payload + off, &gamma_rad, 4); off += 4;
    std::memcpy(payload + off, &pos_downrange, 4); off += 4;
    std::memcpy(payload + off, &pos_crossrange, 4); off += 4;
    std::memcpy(payload + off, &vel_downrange, 4); off += 4;
    std::memcpy(payload + off, &vel_down, 4); off += 4;
    std::memcpy(payload + off, &vel_crossrange, 4); off += 4;
    std::memcpy(payload + off, &bearing_deg, 4); off += 4;
    std::memcpy(payload + off, &target_range_rem, 4); off += 4;
    std::memcpy(payload + off, &iip_lat_e7, 4); off += 4;
    std::memcpy(payload + off, &iip_lon_e7, 4); off += 4;
    std::memcpy(payload + off, &iip_alt, 4); off += 4;
    std::memcpy(payload + off, &mode, 2); off += 2;
    std::memcpy(payload + off, &stage, 1); off += 1;
    std::memcpy(payload + off, &launched, 1); off += 1;
    return off;
}

bool M130GncState::operator==(const M130GncState& rhs) const noexcept {
    return time_usec == rhs.time_usec
        && stage == rhs.stage
        && launched == rhs.launched
        && mode == rhs.mode
        && q_dyn == rhs.q_dyn
        && rho == rhs.rho
        && altitude == rhs.altitude
        && airspeed == rhs.airspeed
        && phi == rhs.phi
        && theta == rhs.theta
        && psi == rhs.psi
        && alpha_est == rhs.alpha_est
        && gamma_rad == rhs.gamma_rad
        && pos_downrange == rhs.pos_downrange
        && pos_crossrange == rhs.pos_crossrange
        && vel_downrange == rhs.vel_downrange
        && vel_down == rhs.vel_down
        && vel_crossrange == rhs.vel_crossrange
        && bearing_deg == rhs.bearing_deg
        && target_range_rem == rhs.target_range_rem
        && iip_lat_e7 == rhs.iip_lat_e7
        && iip_lon_e7 == rhs.iip_lon_e7
        && iip_alt == rhs.iip_alt;
}

bool M130MheDiagnostics::unpack(const std::uint8_t* payload, std::size_t len, M130MheDiagnostics& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.time_usec, payload + off, 8); off += 8;
    std::memcpy(&out.solve_count, payload + off, 4); off += 4;
    std::memcpy(&out.fail_count, payload + off, 4); off += 4;
    std::memcpy(&out.solve_us, payload + off, 4); off += 4;
    std::memcpy(&out.quality, payload + off, 4); off += 4;
    std::memcpy(&out.blend_alpha, payload + off, 4); off += 4;
    std::memcpy(&out.xval_gamma_err, payload + off, 4); off += 4;
    std::memcpy(&out.xval_chi_err, payload + off, 4); off += 4;
    std::memcpy(&out.xval_alt_err, payload + off, 4); off += 4;
    std::memcpy(&out.xval_penalty, payload + off, 4); off += 4;
    std::memcpy(&out.reset_count, payload + off, 4); off += 4;
    std::memcpy(&out.valid, payload + off, 1); off += 1;
    std::memcpy(&out.status, payload + off, 1); off += 1;
    (void) off;
    return true;
}

std::size_t M130MheDiagnostics::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &time_usec, 8); off += 8;
    std::memcpy(payload + off, &solve_count, 4); off += 4;
    std::memcpy(payload + off, &fail_count, 4); off += 4;
    std::memcpy(payload + off, &solve_us, 4); off += 4;
    std::memcpy(payload + off, &quality, 4); off += 4;
    std::memcpy(payload + off, &blend_alpha, 4); off += 4;
    std::memcpy(payload + off, &xval_gamma_err, 4); off += 4;
    std::memcpy(payload + off, &xval_chi_err, 4); off += 4;
    std::memcpy(payload + off, &xval_alt_err, 4); off += 4;
    std::memcpy(payload + off, &xval_penalty, 4); off += 4;
    std::memcpy(payload + off, &reset_count, 4); off += 4;
    std::memcpy(payload + off, &valid, 1); off += 1;
    std::memcpy(payload + off, &status, 1); off += 1;
    return off;
}

bool M130MheDiagnostics::operator==(const M130MheDiagnostics& rhs) const noexcept {
    return time_usec == rhs.time_usec
        && solve_count == rhs.solve_count
        && fail_count == rhs.fail_count
        && solve_us == rhs.solve_us
        && quality == rhs.quality
        && blend_alpha == rhs.blend_alpha
        && xval_gamma_err == rhs.xval_gamma_err
        && xval_chi_err == rhs.xval_chi_err
        && xval_alt_err == rhs.xval_alt_err
        && xval_penalty == rhs.xval_penalty
        && valid == rhs.valid
        && status == rhs.status
        && reset_count == rhs.reset_count;
}

bool M130MpcDiagnostics::unpack(const std::uint8_t* payload, std::size_t len, M130MpcDiagnostics& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.time_usec, payload + off, 8); off += 8;
    std::memcpy(&out.solve_count, payload + off, 4); off += 4;
    std::memcpy(&out.fail_count, payload + off, 4); off += 4;
    std::memcpy(&out.nan_skip_count, payload + off, 4); off += 4;
    std::memcpy(&out.solve_us, payload + off, 4); off += 4;
    std::memcpy(&out.cost_total, payload + off, 4); off += 4;
    std::memcpy(&out.cost_stage, payload + off, 4); off += 4;
    std::memcpy(&out.cost_terminal, payload + off, 4); off += 4;
    std::memcpy(&out.constraint_violation, payload + off, 4); off += 4;
    std::memcpy(&out.delta_roll_cmd, payload + off, 4); off += 4;
    std::memcpy(&out.delta_pitch_cmd, payload + off, 4); off += 4;
    std::memcpy(&out.delta_yaw_cmd, payload + off, 4); off += 4;
    std::memcpy(&out.pitch_accel_cmd, payload + off, 4); off += 4;
    std::memcpy(&out.yaw_accel_cmd, payload + off, 4); off += 4;
    std::memcpy(&out.sqp_iter, payload + off, 2); off += 2;
    std::memcpy(&out.status, payload + off, 2); off += 2;
    (void) off;
    return true;
}

std::size_t M130MpcDiagnostics::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &time_usec, 8); off += 8;
    std::memcpy(payload + off, &solve_count, 4); off += 4;
    std::memcpy(payload + off, &fail_count, 4); off += 4;
    std::memcpy(payload + off, &nan_skip_count, 4); off += 4;
    std::memcpy(payload + off, &solve_us, 4); off += 4;
    std::memcpy(payload + off, &cost_total, 4); off += 4;
    std::memcpy(payload + off, &cost_stage, 4); off += 4;
    std::memcpy(payload + off, &cost_terminal, 4); off += 4;
    std::memcpy(payload + off, &constraint_violation, 4); off += 4;
    std::memcpy(payload + off, &delta_roll_cmd, 4); off += 4;
    std::memcpy(payload + off, &delta_pitch_cmd, 4); off += 4;
    std::memcpy(payload + off, &delta_yaw_cmd, 4); off += 4;
    std::memcpy(payload + off, &pitch_accel_cmd, 4); off += 4;
    std::memcpy(payload + off, &yaw_accel_cmd, 4); off += 4;
    std::memcpy(payload + off, &sqp_iter, 2); off += 2;
    std::memcpy(payload + off, &status, 2); off += 2;
    return off;
}

bool M130MpcDiagnostics::operator==(const M130MpcDiagnostics& rhs) const noexcept {
    return time_usec == rhs.time_usec
        && solve_count == rhs.solve_count
        && fail_count == rhs.fail_count
        && nan_skip_count == rhs.nan_skip_count
        && solve_us == rhs.solve_us
        && sqp_iter == rhs.sqp_iter
        && status == rhs.status
        && cost_total == rhs.cost_total
        && cost_stage == rhs.cost_stage
        && cost_terminal == rhs.cost_terminal
        && constraint_violation == rhs.constraint_violation
        && delta_roll_cmd == rhs.delta_roll_cmd
        && delta_pitch_cmd == rhs.delta_pitch_cmd
        && delta_yaw_cmd == rhs.delta_yaw_cmd
        && pitch_accel_cmd == rhs.pitch_accel_cmd
        && yaw_accel_cmd == rhs.yaw_accel_cmd;
}

bool M130FinState::unpack(const std::uint8_t* payload, std::size_t len, M130FinState& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.time_usec, payload + off, 8); off += 8;
    std::memcpy(&out.fin1_deg, payload + off, 4); off += 4;
    std::memcpy(&out.fin2_deg, payload + off, 4); off += 4;
    std::memcpy(&out.fin3_deg, payload + off, 4); off += 4;
    std::memcpy(&out.fin4_deg, payload + off, 4); off += 4;
    std::memcpy(&out.clamp_count, payload + off, 4); off += 4;
    std::memcpy(&out.offline_events, payload + off, 4); off += 4;
    std::memcpy(&out.servo_online_mask, payload + off, 1); off += 1;
    (void) off;
    return true;
}

std::size_t M130FinState::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &time_usec, 8); off += 8;
    std::memcpy(payload + off, &fin1_deg, 4); off += 4;
    std::memcpy(payload + off, &fin2_deg, 4); off += 4;
    std::memcpy(payload + off, &fin3_deg, 4); off += 4;
    std::memcpy(payload + off, &fin4_deg, 4); off += 4;
    std::memcpy(payload + off, &clamp_count, 4); off += 4;
    std::memcpy(payload + off, &offline_events, 4); off += 4;
    std::memcpy(payload + off, &servo_online_mask, 1); off += 1;
    return off;
}

bool M130FinState::operator==(const M130FinState& rhs) const noexcept {
    return time_usec == rhs.time_usec
        && fin1_deg == rhs.fin1_deg
        && fin2_deg == rhs.fin2_deg
        && fin3_deg == rhs.fin3_deg
        && fin4_deg == rhs.fin4_deg
        && servo_online_mask == rhs.servo_online_mask
        && clamp_count == rhs.clamp_count
        && offline_events == rhs.offline_events;
}

bool M130EventCounters::unpack(const std::uint8_t* payload, std::size_t len, M130EventCounters& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.time_usec, payload + off, 8); off += 8;
    std::memcpy(&out.mpc_fail_count, payload + off, 4); off += 4;
    std::memcpy(&out.mpc_nan_skip_count, payload + off, 4); off += 4;
    std::memcpy(&out.mhe_fail_count, payload + off, 4); off += 4;
    std::memcpy(&out.fin_clamp_count, payload + off, 4); off += 4;
    std::memcpy(&out.xval_reset_count, payload + off, 4); off += 4;
    std::memcpy(&out.servo_offline_events, payload + off, 4); off += 4;
    std::memcpy(&out.link_loss_events, payload + off, 4); off += 4;
    std::memcpy(&out.envelope_violations, payload + off, 4); off += 4;
    (void) off;
    return true;
}

std::size_t M130EventCounters::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &time_usec, 8); off += 8;
    std::memcpy(payload + off, &mpc_fail_count, 4); off += 4;
    std::memcpy(payload + off, &mpc_nan_skip_count, 4); off += 4;
    std::memcpy(payload + off, &mhe_fail_count, 4); off += 4;
    std::memcpy(payload + off, &fin_clamp_count, 4); off += 4;
    std::memcpy(payload + off, &xval_reset_count, 4); off += 4;
    std::memcpy(payload + off, &servo_offline_events, 4); off += 4;
    std::memcpy(payload + off, &link_loss_events, 4); off += 4;
    std::memcpy(payload + off, &envelope_violations, 4); off += 4;
    return off;
}

bool M130EventCounters::operator==(const M130EventCounters& rhs) const noexcept {
    return time_usec == rhs.time_usec
        && mpc_fail_count == rhs.mpc_fail_count
        && mpc_nan_skip_count == rhs.mpc_nan_skip_count
        && mhe_fail_count == rhs.mhe_fail_count
        && fin_clamp_count == rhs.fin_clamp_count
        && xval_reset_count == rhs.xval_reset_count
        && servo_offline_events == rhs.servo_offline_events
        && link_loss_events == rhs.link_loss_events
        && envelope_violations == rhs.envelope_violations;
}

bool M130SensorHealth::unpack(const std::uint8_t* payload, std::size_t len, M130SensorHealth& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.time_usec, payload + off, 8); off += 8;
    std::memcpy(&out.imu_temp_c, payload + off, 4); off += 4;
    std::memcpy(&out.battery_v, payload + off, 4); off += 4;
    std::memcpy(&out.battery_a, payload + off, 4); off += 4;
    std::memcpy(&out.battery_pct, payload + off, 4); off += 4;
    std::memcpy(&out.imu_health, payload + off, 1); off += 1;
    std::memcpy(&out.baro_health, payload + off, 1); off += 1;
    std::memcpy(&out.mag_health, payload + off, 1); off += 1;
    std::memcpy(&out.gps_fix_type, payload + off, 1); off += 1;
    std::memcpy(&out.gps_satellites, payload + off, 1); off += 1;
    std::memcpy(&out.gps_jamming_state, payload + off, 1); off += 1;
    (void) off;
    return true;
}

std::size_t M130SensorHealth::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &time_usec, 8); off += 8;
    std::memcpy(payload + off, &imu_temp_c, 4); off += 4;
    std::memcpy(payload + off, &battery_v, 4); off += 4;
    std::memcpy(payload + off, &battery_a, 4); off += 4;
    std::memcpy(payload + off, &battery_pct, 4); off += 4;
    std::memcpy(payload + off, &imu_health, 1); off += 1;
    std::memcpy(payload + off, &baro_health, 1); off += 1;
    std::memcpy(payload + off, &mag_health, 1); off += 1;
    std::memcpy(payload + off, &gps_fix_type, 1); off += 1;
    std::memcpy(payload + off, &gps_satellites, 1); off += 1;
    std::memcpy(payload + off, &gps_jamming_state, 1); off += 1;
    return off;
}

bool M130SensorHealth::operator==(const M130SensorHealth& rhs) const noexcept {
    return time_usec == rhs.time_usec
        && imu_health == rhs.imu_health
        && baro_health == rhs.baro_health
        && mag_health == rhs.mag_health
        && gps_fix_type == rhs.gps_fix_type
        && gps_satellites == rhs.gps_satellites
        && gps_jamming_state == rhs.gps_jamming_state
        && imu_temp_c == rhs.imu_temp_c
        && battery_v == rhs.battery_v
        && battery_a == rhs.battery_a
        && battery_pct == rhs.battery_pct;
}

bool M130CommandAckM130::unpack(const std::uint8_t* payload, std::size_t len, M130CommandAckM130& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.command_id, payload + off, 4); off += 4;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.progress_pct, payload + off, 2); off += 2;
    std::memcpy(&out.result, payload + off, 1); off += 1;
    std::memcpy(&out.reason_code, payload + off, 1); off += 1;
    (void) off;
    return true;
}

std::size_t M130CommandAckM130::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &command_id, 4); off += 4;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &progress_pct, 2); off += 2;
    std::memcpy(payload + off, &result, 1); off += 1;
    std::memcpy(payload + off, &reason_code, 1); off += 1;
    return off;
}

bool M130CommandAckM130::operator==(const M130CommandAckM130& rhs) const noexcept {
    return command_id == rhs.command_id
        && flight_id == rhs.flight_id
        && result == rhs.result
        && reason_code == rhs.reason_code
        && progress_pct == rhs.progress_pct;
}

bool M130CommandArm::unpack(const std::uint8_t* payload, std::size_t len, M130CommandArm& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.nonce, payload + off, 4); off += 4;
    std::memcpy(out.auth_token, payload + off, 32); off += 32;
    (void) off;
    return true;
}

std::size_t M130CommandArm::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &nonce, 4); off += 4;
    std::memcpy(payload + off, auth_token, 32); off += 32;
    return off;
}

bool M130CommandArm::operator==(const M130CommandArm& rhs) const noexcept {
    return flight_id == rhs.flight_id
        && nonce == rhs.nonce
        && std::memcmp(auth_token, rhs.auth_token, 32) == 0;
}

bool M130CommandDisarm::unpack(const std::uint8_t* payload, std::size_t len, M130CommandDisarm& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.nonce, payload + off, 4); off += 4;
    std::memcpy(out.auth_token, payload + off, 32); off += 32;
    (void) off;
    return true;
}

std::size_t M130CommandDisarm::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &nonce, 4); off += 4;
    std::memcpy(payload + off, auth_token, 32); off += 32;
    return off;
}

bool M130CommandDisarm::operator==(const M130CommandDisarm& rhs) const noexcept {
    return flight_id == rhs.flight_id
        && nonce == rhs.nonce
        && std::memcmp(auth_token, rhs.auth_token, 32) == 0;
}

bool M130CommandHold::unpack(const std::uint8_t* payload, std::size_t len, M130CommandHold& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.nonce, payload + off, 4); off += 4;
    std::memcpy(out.auth_token, payload + off, 32); off += 32;
    (void) off;
    return true;
}

std::size_t M130CommandHold::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &nonce, 4); off += 4;
    std::memcpy(payload + off, auth_token, 32); off += 32;
    return off;
}

bool M130CommandHold::operator==(const M130CommandHold& rhs) const noexcept {
    return flight_id == rhs.flight_id
        && nonce == rhs.nonce
        && std::memcmp(auth_token, rhs.auth_token, 32) == 0;
}

bool M130CommandAbort::unpack(const std::uint8_t* payload, std::size_t len, M130CommandAbort& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.nonce, payload + off, 4); off += 4;
    std::memcpy(&out.reason_code, payload + off, 1); off += 1;
    std::memcpy(out.auth_token, payload + off, 32); off += 32;
    (void) off;
    return true;
}

std::size_t M130CommandAbort::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &nonce, 4); off += 4;
    std::memcpy(payload + off, &reason_code, 1); off += 1;
    std::memcpy(payload + off, auth_token, 32); off += 32;
    return off;
}

bool M130CommandAbort::operator==(const M130CommandAbort& rhs) const noexcept {
    return flight_id == rhs.flight_id
        && nonce == rhs.nonce
        && reason_code == rhs.reason_code
        && std::memcmp(auth_token, rhs.auth_token, 32) == 0;
}

bool M130CommandFts::unpack(const std::uint8_t* payload, std::size_t len, M130CommandFts& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.timestamp_usec, payload + off, 8); off += 8;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.nonce, payload + off, 4); off += 4;
    std::memcpy(&out.command_type, payload + off, 1); off += 1;
    std::memcpy(&out.reason_code, payload + off, 1); off += 1;
    std::memcpy(out.auth_token_rso, payload + off, 32); off += 32;
    std::memcpy(out.auth_token_safety, payload + off, 32); off += 32;
    (void) off;
    return true;
}

std::size_t M130CommandFts::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &timestamp_usec, 8); off += 8;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &nonce, 4); off += 4;
    std::memcpy(payload + off, &command_type, 1); off += 1;
    std::memcpy(payload + off, &reason_code, 1); off += 1;
    std::memcpy(payload + off, auth_token_rso, 32); off += 32;
    std::memcpy(payload + off, auth_token_safety, 32); off += 32;
    return off;
}

bool M130CommandFts::operator==(const M130CommandFts& rhs) const noexcept {
    return flight_id == rhs.flight_id
        && nonce == rhs.nonce
        && timestamp_usec == rhs.timestamp_usec
        && command_type == rhs.command_type
        && reason_code == rhs.reason_code
        && std::memcmp(auth_token_rso, rhs.auth_token_rso, 32) == 0
        && std::memcmp(auth_token_safety, rhs.auth_token_safety, 32) == 0;
}

bool M130CommandTune::unpack(const std::uint8_t* payload, std::size_t len, M130CommandTune& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.nonce, payload + off, 4); off += 4;
    std::memcpy(&out.new_value, payload + off, 4); off += 4;
    std::memcpy(&out.param_id, payload + off, 2); off += 2;
    std::memcpy(out.auth_token, payload + off, 32); off += 32;
    (void) off;
    return true;
}

std::size_t M130CommandTune::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &nonce, 4); off += 4;
    std::memcpy(payload + off, &new_value, 4); off += 4;
    std::memcpy(payload + off, &param_id, 2); off += 2;
    std::memcpy(payload + off, auth_token, 32); off += 32;
    return off;
}

bool M130CommandTune::operator==(const M130CommandTune& rhs) const noexcept {
    return flight_id == rhs.flight_id
        && nonce == rhs.nonce
        && param_id == rhs.param_id
        && new_value == rhs.new_value
        && std::memcmp(auth_token, rhs.auth_token, 32) == 0;
}

bool M130CommandModeSwitch::unpack(const std::uint8_t* payload, std::size_t len, M130CommandModeSwitch& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.nonce, payload + off, 4); off += 4;
    std::memcpy(&out.new_mode, payload + off, 2); off += 2;
    std::memcpy(out.auth_token, payload + off, 32); off += 32;
    (void) off;
    return true;
}

std::size_t M130CommandModeSwitch::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &nonce, 4); off += 4;
    std::memcpy(payload + off, &new_mode, 2); off += 2;
    std::memcpy(payload + off, auth_token, 32); off += 32;
    return off;
}

bool M130CommandModeSwitch::operator==(const M130CommandModeSwitch& rhs) const noexcept {
    return flight_id == rhs.flight_id
        && nonce == rhs.nonce
        && new_mode == rhs.new_mode
        && std::memcmp(auth_token, rhs.auth_token, 32) == 0;
}

bool M130CommandChecklistSign::unpack(const std::uint8_t* payload, std::size_t len, M130CommandChecklistSign& out) noexcept {
    if (!payload || len < kWireSize) return false;
    std::size_t off = 0;
    std::memcpy(&out.flight_id, payload + off, 4); off += 4;
    std::memcpy(&out.item_id, payload + off, 4); off += 4;
    std::memcpy(&out.result, payload + off, 1); off += 1;
    std::memcpy(out.auth_token, payload + off, 32); off += 32;
    (void) off;
    return true;
}

std::size_t M130CommandChecklistSign::pack(std::uint8_t* payload, std::size_t cap) const noexcept {
    if (!payload || cap < kWireSize) return 0;
    std::size_t off = 0;
    std::memcpy(payload + off, &flight_id, 4); off += 4;
    std::memcpy(payload + off, &item_id, 4); off += 4;
    std::memcpy(payload + off, &result, 1); off += 1;
    std::memcpy(payload + off, auth_token, 32); off += 32;
    return off;
}

bool M130CommandChecklistSign::operator==(const M130CommandChecklistSign& rhs) const noexcept {
    return flight_id == rhs.flight_id
        && item_id == rhs.item_id
        && result == rhs.result
        && std::memcmp(auth_token, rhs.auth_token, 32) == 0;
}

} // namespace m130::protocol::gen

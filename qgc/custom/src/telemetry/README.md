# `src/telemetry/` — FactGroups

> **Status**: Foundation scaffold. The concrete Qt FactGroup implementations
> migrate here from `qgc/custom/src/RocketTelemetryFactGroup.{h,cc}` in the
> **Pillar 2 PR** (Protocol Maturity). This directory currently defines the
> target layout and the decomposition plan so downstream consumers can
> already reference the canonical header paths.

## Target decomposition

| FactGroup | Message (m130.xml) | Count | Notes |
|---|---|---|---|
| `GncStateFactGroup`      | M130_GNC_STATE         | ~24 | primary attitude/position/velocity |
| `MheFactGroup`           | M130_MHE_DIAGNOSTICS   | ~14 | estimator health + cross-val |
| `MpcFactGroup`           | M130_MPC_DIAGNOSTICS   | ~16 | controller health + commands |
| `FinFactGroup`           | M130_FIN_STATE         | ~7  | actuator state |
| `EventCountersFactGroup` | M130_EVENT_COUNTERS    | ~9  | counters |
| `SensorHealthFactGroup`  | M130_SENSOR_HEALTH     | ~11 | IMU/GPS/Baro/Mag/Bat |

Migration strategy — carried out in the follow-up PR:
1. Generate skeleton `.h`/`.cc` from `mavlink/m130.xml` via `tools/generate-dialect.py`.
2. Populate `handleMessage()` for the new message ids.
3. Keep `RocketTelemetryFactGroup` (legacy) until dialect cutover is complete.
4. Update `CustomFirmwarePlugin::factGroups()` to expose all six FactGroups
   simultaneously so QML binds to the new hierarchy incrementally.

Each FactGroup continues to obey QGC conventions:
- `_addFact()` in constructor; names match dialect field names 1:1.
- `handleMessage()` is O(1) decode; no allocation on the hot path.
- `vehicle` null-checked where relevant (AGENTS.md Golden Rules).

The existing <ref_file file="/home/ubuntu/_m13/qgc/custom/src/RocketTelemetryFactGroup.h" /> stays in place during Foundation so the current build and HUD keep working while the architecture expands around it.

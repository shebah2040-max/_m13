# `custom/src/` — Source Tree

Two coexisting families during the Foundation period:

1. **Legacy Qt plugin** (`CustomPlugin.cc/h`, `RocketTelemetryFactGroup.cc/h`,
   `FirmwarePlugin/`, `FlyViewCustomLayer.qml`) — the currently-deployed HUD.
   Built by the main QGC CMake flow unchanged. Stays in place until the
   dialect migration (Pillar 2) and view split (Pillar 5) land.

2. **Pure-C++ M130 core** (`safety/`, `logging/`, `access/`, `protocol/`) —
   Qt-free library built and tested by the Foundation CI workflow. Wired into
   `CustomPlugin` in subsequent pillar PRs.

3. **New UI scaffolding** (`views/`, `ui/`, `telemetry/`) — README files and
   QML skeletons defining the target layout. No QML is registered with the
   QGC resource system yet (see `custom.qrc`) — registration happens per-view
   as each pillar is filled in.

## Why two trees coexist during Foundation

The Foundation PR must:
- Not break the existing build.
- Not break the existing HUD behaviour for anyone pulling main.
- Establish the canonical directory structure the rest of the project will
  grow into.

Merging the legacy plugin with the new structure in one PR is not feasible —
that would require full telemetry cutover, view splitting, and Safety Kernel
integration in a single review. Those changes are scoped as their own pillars.

## Migration schedule

| Pillar PR | Migrates |
|---|---|
| Protocol Maturity | `RocketTelemetryFactGroup` → six `src/telemetry/*FactGroup` classes |
| Safety Kernel Integration | wire `MissionStateMachine` + `AlertManager` + `Watchdog` into `CustomPlugin` |
| Views Completion | split `FlyViewCustomLayer.qml` into six Console QML modules |
| Logging Integration | hook `FlightDataRecorder` and `AuditLogger` to live traffic |
| Access Integration | gate `CustomPlugin` commands through `SessionManager` + `CommandAuthorization` |

Every migration step has its own unit + integration tests and regression
coverage for the legacy HUD during the transition.

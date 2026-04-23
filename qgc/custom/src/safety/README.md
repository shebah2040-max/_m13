# `src/safety/` — Safety Kernel

Pure C++ (no Qt GUI deps) safety services. Testable standalone. Wired into
QGC via `CustomPlugin` in a single adapter.

| Class | Purpose | Requirements |
|---|---|---|
| `FlightPhase` | enum mirroring `M130_FLIGHT_PHASE` | SAFE-001 |
| `AlertLevel` | ARINC 661 4-level enum | SAFE-005 |
| `MissionStateMachine` | guarded state machine + transition log | SAFE-001/002 |
| `Watchdog` | multi-channel staleness monitor | SAFE-003, TELE-004, PROT-005/6/7 |
| `AlertManager` | dedup + bounded + escalation-only queue | SAFE-005/006, HAZ-007 |
| `CommandAuthorization` | RBAC + phase + range + rate-limit | CMD-001/004, ACC-001 |
| `FlightSafetyEnvelope` | per-variable per-phase bounds | SAFE-envelope, TELE-003 |
| `FlightTerminationService` | dual-auth FTS dispatcher | SAFE-009/010 |

All classes take injectable `Clock` and sinks for deterministic tests.

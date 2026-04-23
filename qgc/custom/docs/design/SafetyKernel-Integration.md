# Safety Kernel Integration (Pillar 3)

Supersedes the Foundation-era note in
[`SystemArchitecture.md#safety`](SystemArchitecture.md) by describing how
the six pure-C++ Safety Kernel components are wired into the live Qt/QML
GCS plugin, and how MAVLink traffic drives their state.

## Component layout

```
                      ┌──────────────────────────────────────────┐
                      │            CustomPlugin (Qt)             │
                      │                                          │
 MAVLink frame ───────▶ mavlinkMessage(Vehicle*, Link*, msg)     │
                      │   │                                      │
                      │   │  HEARTBEAT → feed("heartbeat")       │
                      │   │  DEBUG_FLOAT_ARRAY(RktGNC)           │
                      │   │     → feed("gnc_state")              │
                      │   │     → evaluateSample(phi/α/mpc_us)   │
                      │   │                                      │
                      │   ▼                                      │
                      │  m130::gui::SafetyKernel (QObject)       │
                      │     ├─ QTimer 5 Hz → tick()              │
                      │     ├─ AlertListModel (QAbstractListModel)
                      │     └─ m130::safety::SafetyAggregator    │
                      │          ├─ MissionStateMachine          │
                      │          ├─ Watchdog                     │
                      │          ├─ AlertManager                 │
                      │          ├─ CommandAuthorization         │
                      │          ├─ FlightSafetyEnvelope         │
                      │          └─ FlightTerminationService     │
                      │                                          │
                      │  QQmlContext.setContextProperty          │
                      │     "m130SafetyKernel" → Safety Kernel   │
                      └──────────────────────────────────────────┘
                                        │
                        QML             ▼
                        ┌──────────────────────────────────────┐
                        │ MasterCautionLight                   │
                        │    level ← m130SafetyKernel.masterLev│
                        │                                      │
                        │ AlertBanner                          │
                        │    model ← m130SafetyKernel.active…  │
                        └──────────────────────────────────────┘
```

## Layered responsibilities

| Layer                | Language | Tested via            | What it owns |
|----------------------|----------|-----------------------|--------------|
| `SafetyAggregator`   | Pure C++ | `tests/core/`         | Routing policy: Watchdog→Alerts, Envelope→Alerts, MissionSM listener, default installers. |
| `SafetyKernel`       | Qt C++   | QGC build + smoke QML | Qt property / signal surface, QTimer-driven tick, ownership of `AlertListModel`. |
| `AlertListModel`     | Qt C++   | QGC build             | `QAbstractListModel` view over `AlertManager::active()`, mapping the pure-C++ `Alert` to QML roles. |
| QML components       | QML      | Manual demo           | Presentation: master lamp, alert banner, acknowledgment action. |

This split keeps the regulatory-relevant safety logic free of Qt — every
single transition, envelope comparison, staleness decision is exercised in
the Qt-free CI job (`tests/core/SafetyAggregatorTest.cc`, plus the component
tests added in the Foundation PR). The Qt wrapper is deliberately
free of decisions: it translates, it does not decide.

## Routing rules (as implemented)

| Input                        | Handler                               | Resulting alert id                 |
|------------------------------|---------------------------------------|------------------------------------|
| `Watchdog::tick()` event     | `SafetyAggregator::wire()` subscriber | `stale.<channel>`                  |
| `FlightSafetyEnvelope` check | `evaluateSample()`                    | `envelope.<variable>`              |
| Command denied               | `evaluateCommand()`                   | `cmd.denied.<command>` (Advisory)  |
| Illegal mission transition   | `requestTransition()`                 | `mission.transition` (Caution)     |

When a channel is fed fresh data or an envelope sample returns nominal,
the matching alert is cleared via `AlertManager::clear(id)` — the lamp and
banner update automatically through the list-model reset.

## MAVLink hook contract

`CustomPlugin::mavlinkMessage` is the single entrypoint. In this PR it
feeds the watchdog from:

- `MAVLINK_MSG_ID_HEARTBEAT` → channel `"heartbeat"`
- `MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY` with `array_id == RocketTelemetryFactGroup::ROCKET_ARRAY_ID`:
  - channel `"gnc_state"`
  - envelope samples for `phi` (idx 18), `alpha_est` (idx 21),
    `mpc_solve_us` (idx 38)

Index mapping matches
[`RocketTelemetryFactGroup.cc`](../../src/RocketTelemetryFactGroup.cc)
and the legacy
[`ICD-DebugFloatArray-legacy.md`](../requirements/ICD-DebugFloatArray-legacy.md);
Pillar 2 will retire this mapping when `m130_gnc_state` is the transport.

## QML context surface

`QQmlContext::setContextProperty("m130SafetyKernel", kernel)` exposes
these bindings to any QML file loaded through the QGC engine:

- `m130SafetyKernel.currentPhase`, `.currentPhaseText`
- `m130SafetyKernel.masterLevel`, `.activeAlertCount`
- `m130SafetyKernel.activeAlerts` (AlertListModel — use as ListView model)
- `m130SafetyKernel.totalAlertsRaised`
- Signals: `phaseChanged`, `masterLevelChanged`, `alertsChanged`,
  `alertRaised(id, level, title)`, `alertAcknowledged(id, user)`
- Methods: `feed(channel)`, `evaluateSample(variable, value)`,
  `requestTransition(phase, reason)`, `acknowledge(id, user)`

## Extending

- **New watchdog channel**: call `safetyKernel()->aggregator().addChannel(...)`
  from `CustomPlugin` once. No QML change required.
- **New envelope variable**: update `FlightSafetyEnvelope::createDefault()`
  (and `docs/safety/SafetyEnvelope.md`) plus add an `evaluateSample` call
  in `mavlinkMessage`.
- **New command**: add a `CommandPolicy` in
  `CommandAuthorization::installDefaultPolicy()` and have the sender pass
  the request through `SafetyKernel::aggregator().evaluateCommand(...)`
  before transmission.

## Non-goals for this PR

- Full six-console UI integration (Pillar 5).
- Persisting the ack journal to the AuditLogger stream (Pillar 6 upgrade
  to HMAC-SHA256 is required before we rely on it for certification).
- Full QML unit tests — Qt Quick Test infrastructure lands with Pillar 5.

## Requirements covered

- **REQ-M130-GCS-SAFE-001/002/003**: Foundation, unchanged (routing is
  now driven by live MAVLink traffic in addition to test harnesses).
- **REQ-M130-GCS-SAFE-004**: Now Integrated — master lamp is bound to a
  running `AlertManager` instance.
- **REQ-M130-GCS-SAFE-005/006**: Foundation; the subscribe sink from the
  aggregator replaces the stub subscriber.
- **REQ-M130-GCS-UI-008**: Master caution is wired to the live kernel.

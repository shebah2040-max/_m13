# Phase B — Qt wrappers + QML consoles + CustomPlugin wiring

Scope: Pillars 5 (Views), 6 (Access), and 7 (Tuning & Analysis) bundled.

## Architecture

Each pillar has a pure-C++ core (covered by `m130_core` stdlib tests) and a
thin Qt-bound facade that exposes `Q_PROPERTY` / `Q_INVOKABLE` APIs to QML.

| Pillar | Pure-C++ core (tests)        | Qt facade (`QObject`)   | QML consumer                          |
|--------|------------------------------|-------------------------|---------------------------------------|
| 5      | `views/*`                    | `MissionController`     | (feeds PreLaunch, RangeSafety, Replay)|
| 6      | `access/*`                   | `AccessController`      | `LoginScreen`, `StepUpDialog`, `AdminConsole` |
| 7a     | `tuning/MpcTuningModel`      | `TuningController`      | `TuningConsole`                       |
| 7b     | `analysis/*`                 | `AnalysisController`    | `AnalysisConsole`                     |

Facades are constructed by `CustomPlugin` and registered as QML context
properties on the engine returned by `createQmlApplicationEngine`:

```
m130SafetyKernel  -> SafetyKernel           (existing from Pillar 3)
m130Access        -> AccessController
m130Mission       -> MissionController
m130Tuning        -> TuningController
m130Analysis      -> AnalysisController
```

## Safety-gate wiring

`TuningController` enforces two independent preconditions before applying a
weight change:

1. **Mission phase** — `phaseAllowsTuning(p)` rejects `BOOST` and `CRUISE`.
   The current phase is pushed from `SafetyKernel::phaseChanged` in
   `CustomPlugin`.
2. **Step-up freshness** — `_step_up_fresh` must be true. It is driven from
   `AccessController::sessionChanged` via `!stepUpRequired()`.

A rejected write returns `SetResult::SafetyGated` from `setParameter()` and
leaves `lastError` populated for the console.

## Live telemetry path

```
MAVLink M130_GNC_STATE
      └─► CustomPlugin::_dispatchM130Message
            ├─► m130GncFactGroup->applyState()  (FactGroup)
            ├─► SafetyKernel::evaluateSample()  (envelope)
            ├─► AnalysisController::push()      (Timeseries per field)
            └─► MissionController::updateIip()  (IIP predictor)
```

The `AnalysisController::push(name, tMs, value)` entry point is used for
eight series by default: `phi`, `theta`, `psi`, `q_dyn`, `altitude`,
`airspeed`, `alpha_est`, `mpc_solve_us`. Additional series can be added at
runtime; missing series are created lazily on first push.

`MHE innovation` is not yet wired from firmware — the `AnalysisConsole`
still visualises the running stats so a test harness or replay stream can
populate it via `pushInnovation(normalised)`.

## QML screens

| File                                    | Binds to               |
|-----------------------------------------|------------------------|
| `views/login/LoginScreen.qml`           | `m130Access`           |
| `views/login/StepUpDialog.qml`          | `m130Access`           |
| `views/tuning/TuningConsole.qml`        | `m130Tuning`, `m130Access` |
| `views/analysis/AnalysisConsole.qml`    | `m130Analysis`         |
| `views/admin/AdminConsole.qml`          | `m130Access`           |

All five screens follow the existing M130 style: `QGCPalette` for colours,
`ScreenTools` for sizing, and the `qrc:/qml/views/...` prefix.

`AnalysisConsole` uses a plain `Canvas` item for the timeseries and
spectrum plots — QtCharts is **not** a dependency of this PR. The peak
frequency is labelled directly on the spectrum canvas.

## Tests

Phase B does not add pure-C++ tests: the underlying cores are already
covered by 35 `m130_core` tests (still 35/35 passing). Qt facade coverage
is deferred to a follow-up PR that introduces `Qt Test` sources once the
CI builds Qt for the custom plugin.

## Deferred (Phase 6b / 7c)

- Argon2id password hasher (needs libsodium)
- `LdapAuthenticator`, `MtlsAuthenticator`
- Live MHE innovation feed from firmware
- Parameter snapshot→FDR append (`TUNING_COMMIT` event)
- Multi-monitor layout persistence

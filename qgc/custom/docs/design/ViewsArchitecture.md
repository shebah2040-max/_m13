# M130 GCS — Views Architecture (Pillar 5)

## 1. Goals

- Replace the single-view HUD with a **console-per-role** layout so Flight
  Director, Range Safety, Operator, and Admin personnel each see only the
  information and controls they need.
- Keep presentation logic in QML and **state logic in pure C++** so it can be
  unit-tested without Qt. QML views should stay binding-only where possible.
- Provide a stable **resource layout** (`qrc:/qml/views/<group>/<Name>.qml`)
  so the switcher, tests, and future plugins can locate consoles without
  hard paths.

## 2. Console catalogue

| Console        | File                                                         | Audience            | Live data today | Planned data wiring |
|----------------|--------------------------------------------------------------|---------------------|-----------------|---------------------|
| Operations     | `src/views/operations/OperationsConsole.qml`                 | Flight Director     | Yes (legacy HUD via Loader) | ADI/tapes migrate to shared widgets in a follow-up PR |
| Pre-Launch     | `src/views/prelaunch/PreLaunchConsole.qml`                   | Ops + Flight Director | Placeholder    | `ChecklistModel` + `CountdownClock` QObject wrappers |
| Range Safety   | `src/views/rangesafety/RangeSafetyConsole.qml`               | Range Safety Officer + Safety Officer | Placeholder | `IipPredictor`, `FlightSafetyEnvelope`, `FlightTerminationService` wrappers |
| Replay         | `src/views/replay/ReplayConsole.qml`                         | Analyst             | Placeholder     | `ReplayController` QObject wrapper around Pillar 4 engine |
| Tuning         | `src/views/tuning/TuningConsole.qml`                         | Controls Engineer   | Placeholder     | `Vehicle.parameterManager` (QGC Fact System) |
| Analysis       | `src/views/analysis/AnalysisConsole.qml`                     | Analyst             | Placeholder     | QtCharts binding (Pillar 7) |
| Admin          | `src/views/admin/AdminConsole.qml`                           | Admin               | Placeholder     | `UserManager`, `SessionManager`, `AuditLogger` wrappers |

A top-level `ConsoleSwitcher.qml` renders a left-side sidebar and hosts the
selected console inside a `Loader`. The switcher is the intended entry
point for a multi-console workspace (CustomPlugin mounts it via
`FlyViewCustomLayer` replacement in a later PR).

## 3. View-model layer (pure C++, tested)

All business logic for the new consoles lives in `src/views/` and is
compiled both into the main QGC binary **and** into the standalone
`m130_core` test library. No Qt / QObject dependencies — Qt wrappers will
be added in a later PR without touching the logic:

| Class | File | Covered by |
|-------|------|------------|
| `ChecklistModel`  | `src/views/ChecklistModel.{h,cc}`  | `tests/core/ChecklistModelTest.cc` |
| `CountdownClock`  | `src/views/CountdownClock.{h,cc}`  | `tests/core/CountdownClockTest.cc` |
| `IipPredictor`    | `src/views/IipPredictor.{h,cc}`    | `tests/core/IipPredictorTest.cc`   |
| `ReplayController`| `src/views/ReplayController.{h,cc}`| `tests/core/ReplayControllerTest.cc` |

### ChecklistModel

- Items have `id`, `title`, `description`, `requires_auth`, `required_role`.
- `markDone` rejects auth-required items without a user or with an
  insufficient role.
- `skip` requires a non-empty reason (MIL-STD-1472H §5.1.10 — rationale
  must be captured for any bypass).
- Blocked items cannot be marked done or skipped; only `reset()` clears
  them.
- `isReadyForLaunch()` is true only when **every** item is `Done` — skipped
  items are an intentional blocker so the final go/no-go is never ambiguous.

### CountdownClock

- Monotonic: every transition driven by `tick(now_ms)`. No internal timer.
- `hold(now_ms)` freezes the clock; `resume(now_ms)` shifts the target
  forward by the hold duration so T-X is preserved.
- `tick()` latches `Launched` when `now_ms >= target_ms` while `Counting`.
- `label()` emits `T-HH:MM:SS`, `T+HH:MM:SS`, `HOLD at T-HH:MM:SS`,
  `LAUNCHED T+HH:MM:SS`, or `—`/`ABORTED` for end states.

### IipPredictor

- First-order ballistic prediction (flat earth, constant g, no drag).
- Returns `valid=false` when the body is below ground with non-positive
  `vz` (discriminant < 0) or when `g <= 0`.
- Intended use: corridor check + emergency cue only. A drag-augmented
  predictor driven by live aerodynamics is deferred to Pillar 9.

### ReplayController

- Wraps `logging::ReplayEngine` (Pillar 4) with an explicit state machine:
  `Empty → Paused → Playing → Finished` with `rewind()` returning to
  `Paused`.
- `play(wall_us)` anchors the engine clock; `pause()` freezes emission
  without touching engine state so resume is exact.
- Sink is preserved across `loadFile()` / `rewind()` so QML sinks stay
  subscribed.

## 4. Shared QML widgets

Reusable building blocks under `src/ui/widgets/`:

- `AdiIndicator.qml` — canonical blue/brown attitude ball with roll scale
  and aircraft reference. Parameters: `pitchDeg`, `rollDeg`, `pixelsPerDeg`.
- `AltitudeTape.qml` — vertical tape, configurable range and major/minor
  tick step (defaults 200 m range, 10 m minor, 50 m major).
- `AirspeedTape.qml` — symmetric tape for m/s with 2 m/s / 10 m/s steps.

## 5. Resource layout

`custom.qrc` exposes the Pillar 5 assets under a single `/qml` prefix so
Loader sources are short and stable:

```
qrc:/qml/ConsoleSwitcher.qml
qrc:/qml/views/<group>/<Name>Console.qml
qrc:/qml/widgets/<Widget>.qml
```

Legacy aliases (`/Custom/qml/...`, `/Custom/qmlimages`, `/custom/img`)
stay untouched — the Operations console still loads the existing HUD via
`qrc:/qml/FlyViewCustomLayer.qml` through a Loader during the transition.

## 6. What is deferred

- 3-D trajectory view + map-based corridor overlay (Pillar 5b).
- Qt/QObject wrappers that expose the four view-models as QML singletons.
- QtCharts binding in the Analysis console (Pillar 7).
- Live parameter editing with safety bounds in the Tuning console
  (Pillar 7 — depends on M130 firmware publishing the parameter set).
- Colour-blind-safe palette variants (Pillar 8).
- Weather / NOTAM / Relay integrations surfaced in the Pre-Launch console
  (Pillar 9).

## 7. Test coverage

The Pillar-4 suite (21 tests) grows by 4:

- `ChecklistModelTest`  — add / auth / skip-with-reason / blocked rejects
  completion / ready-for-launch requires all-done / reset clears state.
- `CountdownClockTest`  — arm rejects past / counts down / hold freezes +
  resume shifts target / tick transitions to Launched / abort is terminal /
  label formatting.
- `IipPredictorTest`    — straight-down / rising-then-falling / invalid
  inputs / rejects zero gravity.
- `ReplayControllerTest`— empty state / load paused by default / play
  emits + EOF / pause halts emission / rewind restarts.

Total: **25 / 25** pure-C++ tests pass. Governance checks
(`validate-dialect.py`, `generate-dialect.py --check`, FDR smoke) remain
green.

# Tuning & Analysis (Pillar 7)

Role: operator-facing analysis primitives (timeseries, FFT, windowed
spectrum, MHE innovation monitor) and the bounded, safety-gated MPC
weight model.

Scope of this PR: pure-C++ cores and unit tests only. Qt/QML wrappers —
QtCharts plot widgets, tuning-console UI with step-up re-auth from
Pillar 6, and FactGroup-fed live streams — land in a follow-up.

Traceability:
REQ-M130-GCS-UI-001 (consoles) / REQ-TELE-003 (envelope alerts) stay at
Foundation; new items reference this doc via the `design/` column.

---

## 1. Goals

1. Provide a **fixed-cost**, testable analysis core that can be fed from
   live telemetry (FactGroup, Pillar 2) or replayed FDR data (Pillar 4)
   with identical semantics.
2. Offer honest, **uncertainty-aware** spectral estimation — windowed
   one-sided magnitude and PSD with coherent-gain and ENBW correction
   per Harris (1978), not a raw FFT magnitude plot.
3. Surface **degradation of the MHE filter** automatically (mean bias,
   variance drift, 3-sigma excursion) so the flight director sees an
   unhealthy estimator before it manifests as an envelope violation.
4. Let an operator tune MPC weights **live** but only within hard-coded
   bounds, with a safety-gate predicate that the integrating PR can wire
   to mission state + session step-up for critical flight phases.

## 2. Components

| Component                                   | File                                         |
|---------------------------------------------|----------------------------------------------|
| `analysis::RingBuffer<T>`                   | `analysis/RingBuffer.h` (header-only)        |
| `analysis::Timeseries`                      | `analysis/Timeseries.{h,cc}`                 |
| `analysis::WindowType` + `makeWindow`       | `analysis/Window.{h,cc}`                     |
| `analysis::fft` / `ifft` / `fftReal`        | `analysis/Fft.{h,cc}`                        |
| `analysis::computeSpectrum` / `findPeak`    | `analysis/Spectrum.{h,cc}`                   |
| `analysis::MheInnovationMonitor`            | `analysis/MheInnovationMonitor.{h,cc}`       |
| `tuning::TuningParameterSpec`, `SetResult`  | `tuning/TuningParameter.h`                   |
| `tuning::MpcTuningModel`, `TuningSnapshot`  | `tuning/MpcTuningModel.{h,cc}`               |

## 3. Timeseries + RingBuffer

`RingBuffer<T>` is a fixed-capacity, header-only circular buffer with
O(1) append and O(1) chronological indexing. `Timeseries` layers a
labelled, time-ordered `Sample { t_ms, value }` on top, with a
non-regression guarantee: timestamps that go backwards are clamped up
to the last seen `t_ms` so downstream FFT / interpolation code can rely
on monotonic time.

`Timeseries::downsample(max_points)` picks up to `max_points` samples
evenly across the current window — cheap and deterministic, suitable for
a 60 Hz plot widget backed by a 10 Hz → 100 Hz telemetry source.
Running `mean`, `stddev`, `minVal`, `maxVal` are available in O(N) over
the current window (the ring is small, typically ≤ 8 192).

## 4. FFT + windows + spectrum

`fft` / `ifft` implement the Cooley-Tukey radix-2 transform in place.
Inputs that are not a power of two are rejected with
`std::invalid_argument`; `fftReal` zero-pads the input up to
`nextPowerOfTwo(x.size())` for convenience. Correctness is checked
against a reference DFT in `FftTest.cc` on a known signal (3 Hz + 5 Hz
mixture at N = 16).

Windows: `Rectangular`, `Hann`, `Hamming`, `Blackman`, `BlackmanHarris`.
`windowCoherentGain` returns Σw/N; `windowEnbw` returns
`N·Σw² / (Σw)²`. Both are used inside `computeSpectrum` to produce:

* `magnitude[k]` — amplitude-corrected, one-sided, with non-DC /
  non-Nyquist bins doubled.
* `psd[k]` — power spectral density in V²/Hz, normalised by
  `fs · N · cg² · enbw`.

`findPeak` returns the dominant bin excluding DC — the obvious
operator-facing number for a vibration tab.

## 5. MHE innovation monitor

Feed the **normalised** innovation sequence `z_k = residual_k /
sqrt(S_kk)`; the monitor maintains running mean and variance in O(1)
space. After `warmup` samples it flags:

* `mean_bias` when `|mean| > mean_sigma / sqrt(N)`. The stderr of the
  sample mean is computed against the **nominal** variance of 1 rather
  than the observed one, so a collapsed-variance sensor (e.g. stuck
  feed) still trips the flag.
* `variance_out` when `|var − 1| > variance_tol` — catches over- or
  under-dispersion (miscalibrated R).
* `three_sigma` on any single `|z_k| > outlier_sigma` — instantaneous
  excursion signal for the UI.

Together these map cleanly to REQ-TELE-003 "safety envelope alerts"
for the estimator and provide input for the `CustomPlugin` safety
aggregator to emit Cautions and Warnings.

## 6. MPC tuning model

`MpcTuningModel` stores named `TuningParameterSpec`s with hard bounds,
unit hint, optional per-call rate limit, and default value. `set` returns
one of `Ok | OutOfRange | RateLimited | SafetyGated | UnknownParameter`
— the operator sees the reason for every rejection rather than a silent
clamp.

`registerStandardMpcWeights` seeds the nine weights used by the MPC
autopilot (`Q_attitude`, `Q_rate`, `Q_accel`, `Q_crossrange`, `R_fin`,
`R_fin_rate`, `Q_terminal`, `mhe_process_q`, `mhe_meas_r`) with
conservative bounds. Raising any bound is a **formal change** and must
go through SCMP review.

### Safety gate

```cpp
model.setSafetyGate([&](std::string_view name, double now, double req) {
    if (missionState != MissionState::Idle &&
        missionState != MissionState::Armed) {
        return false;            // disallowed outside Idle/Armed
    }
    if (session.requiresStepUp(sid, kStepUpMaxAgeMs)) {
        return false;            // must re-auth within N minutes
    }
    return true;
});
```

When the gate returns false, `set` returns `SafetyGated` and the current
value is preserved. Integration with `SessionManager::stepUp` (Pillar 6)
and `MissionStateMachine` (Pillar 3) lands with the Qt wrapper PR.

### Snapshots

`snapshot(label)` returns a timestamped immutable map of parameter
values; `rollback(snap)` restores them, clamping to each parameter's
current bounds in case bounds have been tightened. Snapshots are a
natural payload for a `TUNING_COMMIT` FDR structured event so every
weight change is auditable and replayable.

## 7. Deferred

| Area                                                     | Landing point                              |
|----------------------------------------------------------|--------------------------------------------|
| QtCharts plot widgets + `Timeseries` QObject wrapper     | Pillar 7b (Qt wrapper PR)                  |
| Tuning console UI + Pillar 6 step-up integration          | Pillar 7b / Pillar 8 Admin console        |
| Live FactGroup → `Timeseries`/`MheInnovationMonitor`     | Pillar 7b                                  |
| FDR `TUNING_COMMIT` event + post-flight diff report      | Pillar 4 extension                         |
| Welch / overlap-add PSD for long windows                 | Optional, keep stateless default          |

## 8. Test coverage

41 pure-C++ unit tests in total (29 from Pillar 6 + 6 added here):

* `RingBufferTest` — fill, overwrite, snapshot, clear, out-of-range.
* `TimeseriesTest` — stats, non-monotonic clamp, downsample edge cases.
* `FftTest` — power-of-two helpers, known-vector FFT, DFT reference,
  forward/inverse round-trip, non-power-of-two rejection.
* `SpectrumTest` — window symmetry + gain bounds, rectangular window,
  peak detection on a clean 50 Hz sine, empty/bad-input handling.
* `MheInnovationMonitorTest` — white-noise acceptance, constant-bias
  flag, variance-drift flag, 3-sigma excursion flag, reset.
* `MpcTuningModelTest` — spec validation, standard weight registration,
  out-of-range / NaN / unknown-param rejection, rate limit, safety
  gate, snapshot + rollback (including unknown-key rejection), reset.

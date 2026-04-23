# `custom/tests/` — Test Harness

## Layout

```
tests/
├── core/          # pure-C++ tests for safety/logging/access/protocol (this PR)
├── qml/           # Qt Quick Test (Pillar 5/7)
├── integration/   # SITL-bridge tests (Pillar 9)
└── system/        # full E2E tests with simulated vehicle (Pillar 9)
```

## Running the core suite locally

Requires only a host C++20 toolchain and CMake ≥ 3.22 — Qt is NOT required
because the core library is Qt-free.

```bash
cd qgc/custom/tests/core
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
cd build && ctest --output-on-failure
```

14 Foundation tests currently run under the `M130` and `Unit` CTest labels.
Coverage target is ≥ 80% statement on the Safety Kernel (see `docs/plans/SVP.md`).

## CI integration

- `.github/workflows/m130-foundation.yml` builds and runs the core suite on
  every PR touching `qgc/custom/**`.
- The workflow also runs `tools/validate-traceability.py` and
  `tools/validate-dialect.py` to keep the requirements matrix and MAVLink
  dialect honest.
- The main QGC workflows (`linux.yml` et al.) will pick up the Qt-bound
  FactGroup tests once they migrate out of `src/telemetry/` (Pillar 2 PR).

## Why no Qt Test here yet

The Foundation PR intentionally keeps the core decoupled from Qt so:
1. CI spin-up is fast (~30s vs ~5min for a full Qt build).
2. The Safety Kernel, logging, and access layers are exercised independently
   of UI churn.
3. Regulatory evidence (DO-178C unit tests) is easy to collect without the Qt
   event loop complicating coverage analysis.

The QML and integration harnesses are introduced in their own PRs once the
UI pillars come online.

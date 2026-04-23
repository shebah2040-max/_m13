# M130 GCS — QGroundControl Custom

Ground Control Station for the M130 rocket. Fork of QGroundControl with a
dedicated MAVLink dialect, Safety Kernel, and multi-console UI targeted at
aerospace-grade flight operations.

This directory is organised for **ISO 9001 / AS9100D / DO-278A / DO-178C DAL-C
/ MIL-STD-1472H** evidence collection. Every module has both a traceability
entry in `docs/requirements/traceability.csv` and test coverage linked from
the CSV.

## Tree

```
custom/
├── docs/          regulatory + design documentation
│   ├── requirements/   SRS, ICD, traceability matrix
│   ├── design/         System Architecture
│   ├── plans/          SDP, SVP, SCMP, SQAP
│   ├── safety/         Hazard Log, FMEA, Safety Envelope
│   ├── compliance/     ISO 9001, AS9100D, DO-178C, DO-278A, MIL-STD-1472H, NIST 800-53
│   └── operations/     Pre-Launch Checklist, Post-Flight Report template
├── mavlink/       m130.xml dialect definition
├── src/           C++ + QML sources (see src/README.md)
│   ├── safety/        Safety Kernel (state machine, watchdog, alerts, FTS, envelope, authz)
│   ├── logging/       Audit, Chain of Custody, Flight Data Recorder
│   ├── access/        Role, UserManager, SessionManager
│   ├── protocol/      ProtocolVersion, MessageRouter, M130Dialect
│   ├── telemetry/     (scaffold) target FactGroup layout
│   ├── views/         (scaffold) multi-console QML
│   ├── ui/            Shared QML components (MasterCaution, AlertBanner, theme)
│   ├── CustomPlugin.*       legacy plugin entry point (kept intact)
│   ├── FirmwarePlugin/      legacy PX4-only firmware factory
│   └── RocketTelemetryFactGroup.*  legacy DEBUG_FLOAT_ARRAY FactGroup
├── translations/  Qt Linguist .ts files (ar / en)
├── tests/         test harness (see tests/README.md)
│   └── core/          Qt-free C++ unit tests for the Safety/Logging/Access/Protocol cores
├── tools/         validate-traceability, validate-dialect, post-flight report
├── cmake/         CustomOverrides.cmake (branding, APM disable, icon)
├── res/           images (rocket_icon.svg)
├── custom.qrc     QGC resource override
└── CMakeLists.txt Qt plugin build entry point (unchanged by this PR)
```

## Building

**Full QGC build** (unchanged by this PR; see `qgc/AGENTS.md`):

```bash
cd qgc
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

**Foundation core tests only** (no Qt dependency):

```bash
cd qgc/custom/tests/core
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
cd build && ctest --output-on-failure
```

## Documentation entry points

1. **Start here** — `docs/requirements/SRS-M130GCS.md`
2. **Interfaces** — `docs/requirements/ICD-MAVLink.md`
3. **Architecture** — `docs/design/SystemArchitecture.md`
4. **Plans** — `docs/plans/{SDP,SVP,SCMP,SQAP}.md`
5. **Safety case** — `docs/safety/{HazardLog,FMEA,SafetyEnvelope}.md`
6. **Compliance** — `docs/compliance/*.md`
7. **Operations** — `docs/operations/*.md`

## Roadmap

This PR lays the **Foundation**: directory structure + documentation +
pure-C++ Safety Kernel / logging / access / protocol cores + test harness +
CI. Subsequent PRs flesh out each of the ten pillars:

| Pillar | Scope |
|---|---|
| 1. Requirements & Docs | tighten SRS, add full SCMP baselines, auto-PDF pipeline |
| 2. Protocol Maturity | MAVLink codegen from m130.xml, FactGroup split, dialect cutover |
| 3. Safety Kernel Integration | wire Safety Kernel into `CustomPlugin` |
| 4. Flight Data Recorder | Parquet/Arrow, screenshots, video, replay engine |
| 5. Views Completion | Operations/PreLaunch/RangeSafety consoles |
| 6. Access Integration | local + LDAP auth, mTLS, YubiKey, live audit viewer |
| 7. Tuning & Analysis | MPC tuning UI, plotting, FFT, MHE innovation |
| 8. Admin + Colourblind | admin console, theme engine, i18n completeness |
| 9. System Integration | weather, NOTAMs, MQTT relay, range trackers |
| 10. Certification | ISO 9001 QMS, AS9100D delta, DO-178C evidence, audits |

See `docs/plans/SDP.md` for lifecycle and entry/exit criteria per pillar.

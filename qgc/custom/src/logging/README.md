# `src/logging/` — Flight Data & Audit

| Class | Purpose | Requirements |
|---|---|---|
| `AuditLogger` | append-only, hash-chained audit log | LOG-004, CMD-005, ACC-002 |
| `ChainOfCustody` | MAC-signed file manifests + key rotation | LOG-003, SEC-008 |
| `FlightDataRecorder` | raw MAVLink + JSON events streams | LOG-001/002 |

The Foundation implementation uses a deterministic FNV-1a 64-bit fold as the
integrity function. It is **NOT** a cryptographic MAC. A follow-up PR replaces
this with **HMAC-SHA256 via libsodium** while preserving the API so tests stay
green. All call sites pass through `computeHash()` / `computeMac()` for a
single upgrade point.

Parquet/Arrow, video recording, and screenshot capture land in subsequent
pillar PRs. The current classes provide the persistence surface the views and
protocol layer already rely on, giving us a stable contract for integration.

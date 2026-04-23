---
noteId: "2e8723d13f2e11f1be0e05e75d28fafd"
tags: []

---

# Flight Data Recorder (FDR) — Pillar 4

**Status:** Delivered (core library + live capture + exporters + signed chain of custody + post-flight report).  
**Requirements:** REQ-M130-GCS-LOG-001, -002, -003, -005, -007; REQ-M130-GCS-PROT-004; REQ-M130-GCS-SEC-008.

---

## 1. Goals

- Persist every MAVLink exchange for the mission — raw, human-readable, and decoded — so that flight data can be **replayed**, **re-analysed**, and **audited** long after the mission ends.
- Provide cryptographic integrity (HMAC-SHA256 over each track, SHA-256 per file) so tampering is detectable (aerospace chain of custody).
- Remain **stdlib-only** on both the C++ and Python sides — no OpenSSL, Parquet, or Arrow dependency. This is mandatory to keep CI reproducible and the Custom build self-contained.
- Integrate with `CustomPlugin::mavlinkMessage()` behind a runtime gate (`M130_FDR_DIR` env var) — never surprise a production operator with disk I/O.

Deferred (tracked separately): UI for the Replay console (Pillar 5), Parquet/Arrow bridge (Pillar 7), video/screenshot capture (Pillar 5), TimescaleDB exporter (Pillar 9).

---

## 2. On-disk layout

A single session writes four artefacts rooted at one base path:

```
<base>.m130raw         binary, length-prefixed RawFrame records
<base>.m130events      JSON-lines EventRecord stream
<base>.m130structured  JSON-lines decoded StructuredSample stream
<base>.m130session.json session manifest (counts, per-track SHA-256)
```

### 2.1 Raw record layout (`.m130raw`)

Little-endian, tight — no padding:

| Offset | Size | Field          | Notes                          |
|-------:|-----:|----------------|--------------------------------|
|      0 |   8  | `timestamp_us` | `uint64` wall-clock (or driver)|
|      8 |   1  | `channel`      | 0=telemetry-in 1=command-out 2=event |
|      9 |   2  | `msg_id`       | `uint16`                       |
|     11 |   4  | `payload_len`  | `uint32`                       |
|     15 |  var | `payload[*]`   | raw MAVLink payload bytes      |

A truncated trailing record (power loss) is tolerated on read — `FdrRawReader::next()` returns `nullopt` without failing the stream.

### 2.2 Events (`.m130events`)

Flat JSON-lines. One event per line:

```json
{"ts":1735500000123,"level":"warn","source":"safety.envelope","message":"phi approaching high caution"}
```

### 2.3 Structured samples (`.m130structured`)

Decoded telemetry. Numeric fields go in `n`, string/enum fields in `s`. Ordering is insertion-stable so grep/awk pipelines work:

```json
{"ts_us":12345,"msg_id":42001,"msg":"M130_GNC_STATE","n":{"stage":2,"phi":0.125,"q_dyn":1450.6},"s":{"mode":"CRUISE"}}
```

### 2.4 Session manifest (`.m130session.json`)

Written on `close()`. Never updated afterwards.

```json
{
  "schema_version": "1.0",
  "session_start_us": 123,
  "session_end_us":   9999,
  "counts": {"raw": 4200, "events": 37, "structured": 4200},
  "bytes_written": 185600,
  "tracks": {
    "raw":        {"path": "<base>.m130raw",        "size": 142300, "sha256": "…"},
    "events":     {"path": "<base>.m130events",     "size":   4680, "sha256": "…"},
    "structured": {"path": "<base>.m130structured", "size":  38620, "sha256": "…"}
  }
}
```

---

## 3. Cryptographic integrity

`src/logging/Sha256.{h,cc}` implements FIPS 180-4 SHA-256 from scratch; `src/logging/Hmac.{h,cc}` wraps it per RFC 2104. Test vectors:

| Test                          | Source     |
|-------------------------------|------------|
| SHA-256 empty, "abc", 1M 'a'  | FIPS 180-4 appendix B |
| HMAC-SHA256 cases 1, 2, 3, 6  | RFC 4231   |

`ChainOfCustody` now produces manifests with `mac_alg = "HMAC-SHA256"` and an auxiliary `content_sha256` field. Manifest chains survive key rotation: each entry stores its signing `key_id` and `prev_manifest_hash`.

---

## 4. Runtime integration

```
                 +----------------+      HEARTBEAT / 42000..42255
 MAVLink stack → | CustomPlugin   |────────────────────────┐
                 |  mavlinkMessage|                        ▼
                 +-------+--------+          +--------------------------+
                         │                   | FlightDataRecorder       |
              on success │ isM130Id()        |  appendRaw      (channel 0)
                         ▼                   |  appendStructured (gnc)  |
                 +--------------+             |  appendEvent    (safety)|
                 | _dispatchM130|───────────→ +--------------------------+
                 |   Message    |                              │
                 +------+-------+                              ▼
                        │ gnc                         .m130raw / .m130events
                        ▼                             .m130structured
                  SafetyKernel                        + .m130session.json
```

Recording is **opt-in** via `M130_FDR_DIR`:

```
M130_FDR_DIR=/var/log/m130 ./QGroundControl
```

When the variable is unset, no files are created. The recorder is created per process and closes on destruction (end of QGC lifetime).

---

## 5. Replay engine

`ReplayEngine` drives a `.m130raw` file at `0.1× .. 100×` speed. It is pure C++ (no threading, no sleep) — the host drives it by calling `step(current_wall_us)` from a timer. Typical QGC usage:

```cpp
ReplayEngine eng;
eng.open("/path/to/session.m130raw");
eng.setSpeed(1.0f);
eng.setSink([this](const auto& f) { _feedIntoPlugin(f); });
QTimer::singleShot(0, [&]{ eng.startAt(nowUs()); });
QTimer t; t.start(10ms);
QObject::connect(&t, &QTimer::timeout, [&]{ eng.step(nowUs()); });
```

`rewind()` restarts the file; EOF is observable via `isEof()`.

Limits — by design:
- Speed is clamped: `[0.1, 100.0]`. Out-of-range input is silently clamped.
- Replay preserves order; backwards seeking is not supported.
- Replay does not re-inject frames into the MAVLink bus; the UI layer (Pillar 5) consumes them through the sink.

---

## 6. Post-flight report

`tools/post-flight-report.py` reads the manifest + JSONL tracks and writes Markdown:

```
python3 tools/post-flight-report.py /path/to/session1
# → /path/to/session1.report.md
```

The Markdown is consumed by the compliance pipeline (Sphinx / pandoc) to produce PDF for audit. Stdlib-only → runs anywhere.

---

## 7. Exporters

`FdrExporter::rawToCsv` and `::rawToJsonLines` flatten the raw binary track. CSV header:

```
timestamp_us,channel,msg_id,payload_len,payload_hex
```

Use whichever your downstream tooling prefers (pandas, jq, grep, Excel). The structured JSONL track is itself consumable directly.

---

## 8. Tests

All tests are pure C++ and run under `tests/core` (no Qt, no external libs):

| Suite                     | Assertions                                                  |
|---------------------------|-------------------------------------------------------------|
| `Sha256Test`              | 5 NIST-style vectors                                         |
| `HmacSha256Test`          | 5 RFC 4231 vectors (cases 1/2/3/6 + incremental vs one-shot) |
| `ChainOfCustodyTest`      | sign/verify + tamper + chain linking + key rotation          |
| `FlightDataRecorderTest`  | all 3 tracks + session manifest + SHA-256 present            |
| `FdrReaderTest`           | raw round-trip, events round-trip, truncated-tail tolerance  |
| `ReplayEngineTest`        | 1×, 10×, 0.5×, speed clamping, rewind                        |
| `FdrExporterTest`         | CSV + JSONL export                                           |

Total: 21/21 tests green.

---

## 9. Limitations & follow-ups

- No streaming Parquet/Arrow output — deferred to Pillar 7 (analysis) where the toolchain can be added.
- No in-tree video/screenshot capture yet — deferred to Pillar 5 so QML/test scaffolding is available.
- Replay does not yet reconstruct decoded `StructuredSample`s from `.m130raw`; the raw track is replayed as-is. Add a generated-dialect unpacker-on-replay in a follow-up.
- `ChainOfCustody` signs files; it does not sign log streams incrementally. Streaming HMAC on the hot path is a future optimisation.

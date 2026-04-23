# `custom/tools/` — Build & Governance Helpers

Python helpers for CI and local development. All tools use the stdlib only so
they run during the earliest CI bootstrap stage.

| Tool | Purpose |
|---|---|
| `validate-traceability.py` | verifies every `REQ-M130-GCS-*` id referenced in docs exists in `traceability.csv` and every CSV row maps to real files |
| `validate-dialect.py` | syntactic validation of `mavlink/m130.xml` (well-formed, id ranges, duplicate names) |
| `post-flight-report-generator.py` | renders `docs/operations/PostFlightReport-template.md` with data pulled from an FDR directory |

Run all:

```bash
python3 qgc/custom/tools/validate-traceability.py
python3 qgc/custom/tools/validate-dialect.py
```

Both return exit code 0 on success and non-zero on violations, suitable for
inclusion in the Foundation CI workflow (`.github/workflows/m130-foundation.yml`).

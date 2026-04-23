#!/usr/bin/env python3
"""Smoke test: feed a synthetic FDR session into tools/post-flight-report.py
and verify a non-empty Markdown report is produced. Used by CI to guard the
post-flight tooling without needing to run the C++ FDR writer.
"""
from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.path.abspath(os.path.join(HERE, os.pardir, "post-flight-report.py"))


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        base = os.path.join(tmp, "session")
        manifest = {
            "schema_version": "1.0",
            "session_start_us": 1,
            "session_end_us": 1000,
            "counts": {"raw": 0, "events": 1, "structured": 1},
            "bytes_written": 0,
            "tracks": {
                "raw":        {"path": base + ".m130raw",        "size": 0, "sha256": "00" * 32},
                "events":     {"path": base + ".m130events",     "size": 0, "sha256": "00" * 32},
                "structured": {"path": base + ".m130structured", "size": 0, "sha256": "00" * 32},
            },
        }
        with open(base + ".m130session.json", "w") as fh:
            json.dump(manifest, fh)
        with open(base + ".m130events", "w") as fh:
            fh.write('{"ts":1,"level":"info","source":"test","message":"ok"}\n')
        with open(base + ".m130structured", "w") as fh:
            fh.write('{"ts_us":1,"msg_id":42001,"msg":"T","n":{"q_dyn":1.0},"s":{}}\n')
        open(base + ".m130raw", "wb").close()

        subprocess.check_call([sys.executable, TOOL, base])
        report = base + ".report.md"
        if not os.path.exists(report) or os.path.getsize(report) == 0:
            print(f"FAIL: {report} missing or empty", file=sys.stderr)
            return 1
        with open(report, encoding="utf-8") as fh:
            text = fh.read()
        for needle in ("# M130 Post-Flight Report", "## Session", "## Track integrity"):
            if needle not in text:
                print(f"FAIL: missing '{needle}' in report", file=sys.stderr)
                return 2
    print("OK: post-flight report generated")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

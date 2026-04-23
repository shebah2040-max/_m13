#!/usr/bin/env python3
"""Render a post-flight report from an FDR directory.

Foundation scope: skeletal — takes a template and an FDR directory, produces
a filled Markdown report. The full implementation (event timeline, plots,
compliance verdict) lands in Pillar 4.

Usage:
  post-flight-report-generator.py --fdr <dir> --template <md> --out <md>
"""
from __future__ import annotations

import argparse
import datetime
import json
import pathlib
import sys


def parse_events(path: pathlib.Path) -> list[dict]:
    if not path.exists():
        return []
    out: list[dict] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            out.append(json.loads(line))
        except json.JSONDecodeError:
            continue
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--fdr", type=pathlib.Path, required=True)
    parser.add_argument("--template", type=pathlib.Path, required=True)
    parser.add_argument("--out", type=pathlib.Path, required=True)
    args = parser.parse_args()

    if not args.fdr.is_dir():
        print(f"fdr: {args.fdr} is not a directory", file=sys.stderr)
        return 1
    if not args.template.exists():
        print(f"template: {args.template} missing", file=sys.stderr)
        return 1

    # Minimal templating: fill {{FIELDS}} tokens with stub values so tests and
    # pipelines can exercise the full pipeline. Real fill lands in Pillar 4.
    template = args.template.read_text(encoding="utf-8")

    events_path = next(args.fdr.glob("*.m130events"), None)
    events = parse_events(events_path) if events_path else []

    fills = {
        "GENERATED_AT": datetime.datetime.utcnow().isoformat(timespec="seconds"),
        "FDR_DIR": str(args.fdr),
        "EVENT_COUNT": str(len(events)),
        "TEMPLATE_NOTE": "Foundation skeleton — full fill lands in Pillar 4",
    }

    rendered = template
    for k, v in fills.items():
        rendered = rendered.replace("{{" + k + "}}", v)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(rendered, encoding="utf-8")
    print(f"post-flight-report-generator: wrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Validate docs/requirements/traceability.csv against the documentation tree.

Checks:
  1. Every row has the eight expected columns.
  2. Every REQ id is unique.
  3. Every DesignRef / CodeRef / TestRef path resolves to a real file
     (fragments after '#' are allowed and ignored for existence checks).

Exits 0 on success, 1 on the first violation.
"""
from __future__ import annotations

import csv
import pathlib
import sys


REPO_ROOT = pathlib.Path(__file__).resolve().parents[3]
CUSTOM = REPO_ROOT / "qgc" / "custom"
CSV_PATH = CUSTOM / "docs" / "requirements" / "traceability.csv"

EXPECTED_COLUMNS = [
    "RequirementID",
    "Category",
    "Title",
    "DesignRef",
    "CodeRef",
    "TestRef",
    "VerificationMethod",
    "Status",
]


def fail(msg: str) -> int:
    print(f"validate-traceability: FAIL: {msg}", file=sys.stderr)
    return 1


def resolves(ref: str) -> bool:
    """Accept empty, TBD, or a path relative to either ``custom/`` or ``custom/docs/``.

    Design references are typically ``design/SystemArchitecture.md#anchor``
    (relative to ``docs/``). Code / test references are ``src/...`` or
    ``tests/...`` (relative to ``custom/``). Both layouts are valid.
    """
    if not ref or ref in {"TBD", "-", "N/A", "CI", "ci", "CODEOWNERS"}:
        return True
    path_part = ref.split("#", 1)[0].strip()
    if not path_part:
        return True
    for base in (CUSTOM, CUSTOM / "docs"):
        if (base / path_part).exists():
            return True
    return False


def main() -> int:
    if not CSV_PATH.exists():
        return fail(f"missing {CSV_PATH}")

    with CSV_PATH.open(newline="", encoding="utf-8") as fh:
        reader = csv.reader(fh)
        rows = list(reader)

    if not rows:
        return fail("empty traceability.csv")

    header = rows[0]
    if header != EXPECTED_COLUMNS:
        return fail(
            f"unexpected header {header}, expected {EXPECTED_COLUMNS}"
        )

    seen_ids: set[str] = set()
    errors: list[str] = []

    for lineno, row in enumerate(rows[1:], start=2):
        if len(row) != len(EXPECTED_COLUMNS):
            errors.append(f"line {lineno}: wrong column count ({len(row)})")
            continue
        req_id = row[0].strip()
        if not req_id.startswith("REQ-M130-GCS-"):
            errors.append(f"line {lineno}: REQ id '{req_id}' malformed")
        if req_id in seen_ids:
            errors.append(f"line {lineno}: duplicate REQ id '{req_id}'")
        seen_ids.add(req_id)

        status = row[7].strip()
        # Rows marked "Pending" target future PRs and may reference files
        # that do not yet exist. Existence is only enforced on rows claimed
        # as Foundation / Implemented / Verified.
        enforce_existence = status not in {"Pending", "Planned", "Deferred"}

        for col_name, ref in zip(EXPECTED_COLUMNS[3:6], row[3:6]):
            ref = ref.strip()
            if enforce_existence and not resolves(ref):
                errors.append(
                    f"line {lineno} {col_name}: '{ref}' does not resolve"
                )

    if errors:
        for e in errors:
            print(f"validate-traceability: {e}", file=sys.stderr)
        return 1

    print(f"validate-traceability: OK ({len(rows) - 1} requirements)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

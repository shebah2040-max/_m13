#!/usr/bin/env python3
"""Validate custom/mavlink/m130.xml structure.

Checks:
  1. XML is well-formed.
  2. All <message id="…"> are integers in 42000–42255 (M130 reserved block).
  3. No duplicate message ids.
  4. No duplicate message names.
  5. Every <enum> referenced by a field has a definition.
"""
from __future__ import annotations

import pathlib
import re
import sys
import xml.etree.ElementTree as ET


REPO_ROOT = pathlib.Path(__file__).resolve().parents[3]
XML_PATH = REPO_ROOT / "qgc" / "custom" / "mavlink" / "m130.xml"


def fail(msg: str) -> int:
    print(f"validate-dialect: FAIL: {msg}", file=sys.stderr)
    return 1


def main() -> int:
    if not XML_PATH.exists():
        return fail(f"missing {XML_PATH}")

    try:
        tree = ET.parse(XML_PATH)
    except ET.ParseError as e:
        return fail(f"XML parse error: {e}")

    root = tree.getroot()
    errors: list[str] = []

    # Collect enums.
    enum_names: set[str] = {
        e.get("name", "") for e in root.iter("enum")
    }

    # Check messages.
    seen_ids: set[int] = set()
    seen_names: set[str] = set()

    for msg in root.iter("message"):
        mid = msg.get("id")
        name = msg.get("name", "")
        if not mid or not name:
            errors.append("message missing id or name")
            continue
        try:
            mid_i = int(mid)
        except ValueError:
            errors.append(f"message {name}: id '{mid}' not integer")
            continue
        if not (42000 <= mid_i <= 42255):
            errors.append(
                f"message {name}: id {mid_i} outside 42000-42255"
            )
        if mid_i in seen_ids:
            errors.append(f"duplicate message id {mid_i}")
        if name in seen_names:
            errors.append(f"duplicate message name {name}")
        seen_ids.add(mid_i)
        seen_names.add(name)

        # Validate field enum refs.
        for field in msg.iter("field"):
            enum = field.get("enum")
            if enum and enum not in enum_names:
                errors.append(
                    f"{name}.{field.get('name')}: unknown enum '{enum}'"
                )

    # Require at least one message.
    if not seen_ids:
        errors.append("no messages defined")

    if errors:
        for e in errors:
            print(f"validate-dialect: {e}", file=sys.stderr)
        return 1

    print(
        f"validate-dialect: OK ({len(seen_ids)} messages, "
        f"{len(enum_names)} enums)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

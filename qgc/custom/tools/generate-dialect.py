#!/usr/bin/env python3
"""Generate C++17 headers/sources from qgc/custom/mavlink/m130.xml.

Outputs (committed, regenerated on demand):
  src/protocol/generated/M130Enums.generated.h
  src/protocol/generated/M130Messages.generated.h
  src/protocol/generated/M130Messages.generated.cc
  src/protocol/generated/M130DialectTable.generated.h
  src/protocol/generated/M130DialectTable.generated.cc

Field layout matches MAVLink v2 payload ordering: stable-sort by
descending primitive size (uint64_t=8, float/uint32_t=4, uint16_t=2,
uint8_t=1). Strings are treated as ``uint8_t[N]``. Fields with no
explicit array size are scalar.

Usage:
  python3 tools/generate-dialect.py             # write files
  python3 tools/generate-dialect.py --check     # fail if files differ

Stdlib-only (runs pre-dependency in CI).
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys
import xml.etree.ElementTree as ET
from typing import Iterable, NamedTuple


REPO_ROOT = pathlib.Path(__file__).resolve().parents[3]
CUSTOM = REPO_ROOT / "qgc" / "custom"
XML_PATH = CUSTOM / "mavlink" / "m130.xml"
OUT_DIR = CUSTOM / "src" / "protocol" / "generated"

GENERATED_BANNER = (
    "// SPDX-License-Identifier: GPL-3.0-or-later\n"
    "// AUTO-GENERATED from qgc/custom/mavlink/m130.xml by tools/generate-dialect.py.\n"
    "// DO NOT EDIT. Re-run the generator when the dialect changes.\n"
)


# ---- Type model --------------------------------------------------------

_PRIMITIVE_SIZE = {
    "uint8_t": 1,
    "int8_t": 1,
    "uint16_t": 2,
    "int16_t": 2,
    "uint32_t": 4,
    "int32_t": 4,
    "uint64_t": 8,
    "int64_t": 8,
    "float": 4,
    "double": 8,
    "char": 1,
}


class FieldSpec(NamedTuple):
    name: str
    primitive: str        # e.g. "uint32_t"
    array_len: int        # 0 for scalar, >0 for arrays
    enum_name: str        # empty string when not an enum
    units: str
    description: str

    @property
    def primitive_size(self) -> int:
        return _PRIMITIVE_SIZE[self.primitive]

    @property
    def wire_size(self) -> int:
        n = max(self.array_len, 1)
        return self.primitive_size * n

    @property
    def cpp_scalar_type(self) -> str:
        if self.primitive == "float":
            return "float"
        if self.primitive == "double":
            return "double"
        if self.primitive == "char":
            return "char"
        return f"std::{self.primitive}"

    @property
    def cpp_decl(self) -> str:
        t = self.cpp_scalar_type
        if self.array_len > 0:
            return f"{t} {self.name}[{self.array_len}]"
        return f"{t} {self.name}"


class MessageSpec(NamedTuple):
    msg_id: int
    name: str
    camel: str           # e.g. "HeartbeatExtended"
    description: str
    fields: list[FieldSpec]       # declaration order (as authored)
    wire_fields: list[FieldSpec]  # wire order (sorted by size desc)
    inbound: bool
    rate_hz: float


class EnumEntry(NamedTuple):
    name: str
    value: int
    description: str


class EnumSpec(NamedTuple):
    name: str
    cpp_name: str   # e.g. "M130FlightPhase"
    description: str
    entries: list[EnumEntry]


# ---- XML parsing -------------------------------------------------------


_ARRAY_RE = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*)\[(\d+)\]$")


def _parse_field_type(raw: str) -> tuple[str, int]:
    raw = raw.strip()
    m = _ARRAY_RE.match(raw)
    if m:
        return m.group(1), int(m.group(2))
    return raw, 0


def _parse_field_tag(elem: ET.Element) -> FieldSpec:
    raw_type = elem.get("type", "").strip()
    raw_name = elem.get("name", "").strip()
    primitive, array_len = _parse_field_type(raw_type)
    # XML permits the array form in the name too: "auth_token[32]".
    m = _ARRAY_RE.match(raw_name)
    if m:
        raw_name = m.group(1)
        array_len = int(m.group(2))
    if primitive not in _PRIMITIVE_SIZE:
        raise ValueError(f"unknown primitive '{primitive}' in field '{raw_name}'")
    return FieldSpec(
        name=raw_name,
        primitive=primitive,
        array_len=array_len,
        enum_name=elem.get("enum", "").strip(),
        units=elem.get("units", "").strip(),
        description=(elem.text or "").strip(),
    )


_INBOUND_RATE = {
    "M130_HEARTBEAT_EXTENDED": (True, 1.0),
    "M130_GNC_STATE": (True, 20.0),
    "M130_MHE_DIAGNOSTICS": (True, 10.0),
    "M130_MPC_DIAGNOSTICS": (True, 10.0),
    "M130_FIN_STATE": (True, 20.0),
    "M130_EVENT_COUNTERS": (True, 1.0),
    "M130_SENSOR_HEALTH": (True, 1.0),
    "M130_COMMAND_ACK_M130": (True, 0.0),
}


def _to_camel(snake: str) -> str:
    """M130_GNC_STATE -> M130GncState, M130_COMMAND_ARM -> M130CommandArm."""
    parts = snake.split("_")
    return "".join(p[:1].upper() + p[1:].lower() if p else "" for p in parts)


def _enum_cpp_name(xml_name: str) -> str:
    # M130_FLIGHT_PHASE -> M130FlightPhase
    return _to_camel(xml_name)


def _parse_xml() -> tuple[list[EnumSpec], list[MessageSpec]]:
    tree = ET.parse(XML_PATH)
    root = tree.getroot()

    enums: list[EnumSpec] = []
    for e in root.iter("enum"):
        name = e.get("name", "")
        desc_elem = e.find("description")
        desc = (desc_elem.text or "").strip() if desc_elem is not None else ""
        entries: list[EnumEntry] = []
        for entry in e.iter("entry"):
            try:
                value = int(entry.get("value", "0"))
            except ValueError as err:
                raise ValueError(
                    f"enum {name}: entry '{entry.get('name')}' value not int"
                ) from err
            desc_e = entry.find("description")
            entries.append(
                EnumEntry(
                    name=entry.get("name", ""),
                    value=value,
                    description=(
                        desc_e.text or ""
                    ).strip() if desc_e is not None else "",
                )
            )
        enums.append(
            EnumSpec(
                name=name,
                cpp_name=_enum_cpp_name(name),
                description=desc,
                entries=entries,
            )
        )

    messages: list[MessageSpec] = []
    for msg in root.iter("message"):
        name = msg.get("name", "")
        try:
            msg_id = int(msg.get("id", "-1"))
        except ValueError as err:
            raise ValueError(f"message {name}: id not int") from err
        desc_elem = msg.find("description")
        desc = (desc_elem.text or "").strip() if desc_elem is not None else ""

        fields: list[FieldSpec] = []
        for f in msg.iter("field"):
            fields.append(_parse_field_tag(f))

        # MAVLink v2 wire order: stable sort by primitive size desc.
        wire_fields = sorted(fields, key=lambda f: -f.primitive_size)

        inbound, rate = _INBOUND_RATE.get(name, (False, 0.0))
        messages.append(
            MessageSpec(
                msg_id=msg_id,
                name=name,
                camel=_to_camel(name),
                description=desc,
                fields=fields,
                wire_fields=wire_fields,
                inbound=inbound,
                rate_hz=rate,
            )
        )

    messages.sort(key=lambda m: m.msg_id)
    return enums, messages


# ---- Emitters ----------------------------------------------------------


def _wire_size(m: MessageSpec) -> int:
    return sum(f.wire_size for f in m.wire_fields)


def _emit_enums_header(enums: list[EnumSpec]) -> str:
    out: list[str] = [
        GENERATED_BANNER,
        "#pragma once",
        "",
        "#include <cstdint>",
        "",
        "namespace m130::protocol::gen {",
        "",
    ]
    for e in enums:
        if e.description:
            out.append(f"/// {e.description}")
        out.append(f"enum class {e.cpp_name} : std::uint32_t {{")
        for entry in e.entries:
            if entry.description:
                out.append(f"    /// {entry.description}")
            out.append(f"    {entry.name} = {entry.value},")
        out.append("};")
        out.append("")
        out.append(f"constexpr const char* toString({e.cpp_name} v) noexcept {{")
        out.append("    switch (v) {")
        for entry in e.entries:
            out.append(
                f"        case {e.cpp_name}::{entry.name}: return \"{entry.name}\";"
            )
        out.append("    }")
        out.append("    return \"?\";")
        out.append("}")
        out.append("")
    out.append("} // namespace m130::protocol::gen")
    out.append("")
    return "\n".join(out)


def _emit_messages_header(messages: list[MessageSpec]) -> str:
    out: list[str] = [
        GENERATED_BANNER,
        "#pragma once",
        "",
        "#include \"M130Enums.generated.h\"",
        "",
        "#include <array>",
        "#include <cstddef>",
        "#include <cstdint>",
        "",
        "namespace m130::protocol::gen {",
        "",
    ]
    for m in messages:
        if m.description:
            out.append(f"/// msgid {m.msg_id} — {m.description}")
        out.append(f"struct {m.camel} {{")
        out.append(f"    static constexpr std::uint32_t kMsgId = {m.msg_id};")
        out.append(
            f"    static constexpr std::size_t kWireSize = {_wire_size(m)};"
        )
        out.append(f"    static constexpr const char* kName = \"{m.name}\";")
        out.append("")
        for f in m.fields:
            comment_parts = []
            if f.units:
                comment_parts.append(f"units={f.units}")
            if f.enum_name:
                comment_parts.append(f"enum={f.enum_name}")
            if f.description:
                comment_parts.append(f.description.replace("\n", " "))
            comment = f"  ///< {' | '.join(comment_parts)}" if comment_parts else ""
            out.append(f"    {f.cpp_decl} = {{}};{comment}")
        out.append("")
        out.append(
            f"    static bool unpack(const std::uint8_t* payload, std::size_t len, {m.camel}& out) noexcept;"
        )
        out.append(
            f"    std::size_t pack(std::uint8_t* payload, std::size_t cap) const noexcept;"
        )
        out.append(f"    bool operator==(const {m.camel}& rhs) const noexcept;")
        out.append(f"    bool operator!=(const {m.camel}& rhs) const noexcept {{ return !(*this == rhs); }}")
        out.append("};")
        out.append("")
    out.append("} // namespace m130::protocol::gen")
    out.append("")
    return "\n".join(out)


def _emit_unpack_body(m: MessageSpec) -> list[str]:
    lines: list[str] = [
        "    if (!payload || len < kWireSize) return false;",
        "    std::size_t off = 0;",
    ]
    for f in m.wire_fields:
        if f.array_len > 0:
            # memcpy the whole array
            lines.append(
                f"    std::memcpy(out.{f.name}, payload + off, {f.wire_size}); off += {f.wire_size};"
            )
        else:
            lines.append(
                f"    std::memcpy(&out.{f.name}, payload + off, {f.primitive_size}); off += {f.primitive_size};"
            )
    lines.append("    (void) off;")
    lines.append("    return true;")
    return lines


def _emit_pack_body(m: MessageSpec) -> list[str]:
    lines: list[str] = [
        "    if (!payload || cap < kWireSize) return 0;",
        "    std::size_t off = 0;",
    ]
    for f in m.wire_fields:
        if f.array_len > 0:
            lines.append(
                f"    std::memcpy(payload + off, {f.name}, {f.wire_size}); off += {f.wire_size};"
            )
        else:
            lines.append(
                f"    std::memcpy(payload + off, &{f.name}, {f.primitive_size}); off += {f.primitive_size};"
            )
    lines.append("    return off;")
    return lines


def _emit_equals_body(m: MessageSpec) -> list[str]:
    parts: list[str] = []
    for f in m.fields:
        if f.array_len > 0:
            parts.append(
                f"std::memcmp({f.name}, rhs.{f.name}, {f.wire_size}) == 0"
            )
        else:
            parts.append(f"{f.name} == rhs.{f.name}")
    if not parts:
        return ["    return true;"]
    return [
        "    return " + "\n        && ".join(parts) + ";",
    ]


def _emit_messages_source(messages: list[MessageSpec]) -> str:
    out: list[str] = [
        GENERATED_BANNER,
        "#include \"M130Messages.generated.h\"",
        "",
        "#include <cstring>",
        "",
        "namespace m130::protocol::gen {",
        "",
    ]
    for m in messages:
        out.append(
            f"bool {m.camel}::unpack(const std::uint8_t* payload, std::size_t len, {m.camel}& out) noexcept {{"
        )
        out.extend(_emit_unpack_body(m))
        out.append("}")
        out.append("")
        out.append(
            f"std::size_t {m.camel}::pack(std::uint8_t* payload, std::size_t cap) const noexcept {{"
        )
        out.extend(_emit_pack_body(m))
        out.append("}")
        out.append("")
        out.append(
            f"bool {m.camel}::operator==(const {m.camel}& rhs) const noexcept {{"
        )
        out.extend(_emit_equals_body(m))
        out.append("}")
        out.append("")
    out.append("} // namespace m130::protocol::gen")
    out.append("")
    return "\n".join(out)


def _emit_table_header(messages: list[MessageSpec]) -> str:
    out: list[str] = [
        GENERATED_BANNER,
        "#pragma once",
        "",
        "#include <array>",
        "#include <cstddef>",
        "#include <cstdint>",
        "",
        "namespace m130::protocol::gen {",
        "",
        "struct DialectEntry {",
        "    std::uint32_t msg_id;",
        "    const char*   name;",
        "    std::size_t   wire_size;",
        "    bool          inbound;",
        "    float         rate_hz;",
        "};",
        "",
        f"constexpr std::size_t kDialectMessageCount = {len(messages)};",
        "",
        "extern const std::array<DialectEntry, kDialectMessageCount> kDialectTable;",
        "",
        "/// Find a dialect entry by id. Returns nullptr if unknown.",
        "const DialectEntry* findDialectEntry(std::uint32_t msg_id) noexcept;",
        "",
        "} // namespace m130::protocol::gen",
        "",
    ]
    return "\n".join(out)


def _emit_table_source(messages: list[MessageSpec]) -> str:
    out: list[str] = [
        GENERATED_BANNER,
        "#include \"M130DialectTable.generated.h\"",
        "#include \"M130Messages.generated.h\"",
        "",
        "namespace m130::protocol::gen {",
        "",
        "const std::array<DialectEntry, kDialectMessageCount> kDialectTable{{",
    ]
    for m in messages:
        out.append(
            "    {%d, \"%s\", %d, %s, %.1ff},"
            % (
                m.msg_id,
                m.name,
                _wire_size(m),
                "true" if m.inbound else "false",
                m.rate_hz,
            )
        )
    out.extend(
        [
            "}};",
            "",
            "const DialectEntry* findDialectEntry(std::uint32_t msg_id) noexcept {",
            "    for (const auto& e : kDialectTable) {",
            "        if (e.msg_id == msg_id) return &e;",
            "    }",
            "    return nullptr;",
            "}",
            "",
            "} // namespace m130::protocol::gen",
            "",
        ]
    )
    return "\n".join(out)


# ---- Write / check ----------------------------------------------------


def _write_if_changed(path: pathlib.Path, content: str, check: bool) -> bool:
    path.parent.mkdir(parents=True, exist_ok=True)
    existing = path.read_text(encoding="utf-8") if path.exists() else ""
    if existing == content:
        return False
    if check:
        print(
            f"generate-dialect: FAIL: {path.relative_to(REPO_ROOT)} is stale — "
            f"re-run tools/generate-dialect.py",
            file=sys.stderr,
        )
        return True
    path.write_text(content, encoding="utf-8")
    print(f"generate-dialect: wrote {path.relative_to(REPO_ROOT)}")
    return True


def main(argv: Iterable[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="exit non-zero if generated files differ from XML",
    )
    args = parser.parse_args(list(argv))

    if not XML_PATH.exists():
        print(f"generate-dialect: missing {XML_PATH}", file=sys.stderr)
        return 1

    enums, messages = _parse_xml()

    outputs = [
        (OUT_DIR / "M130Enums.generated.h", _emit_enums_header(enums)),
        (OUT_DIR / "M130Messages.generated.h", _emit_messages_header(messages)),
        (OUT_DIR / "M130Messages.generated.cc", _emit_messages_source(messages)),
        (
            OUT_DIR / "M130DialectTable.generated.h",
            _emit_table_header(messages),
        ),
        (
            OUT_DIR / "M130DialectTable.generated.cc",
            _emit_table_source(messages),
        ),
    ]

    stale = False
    for path, content in outputs:
        if _write_if_changed(path, content, args.check):
            stale = True

    if args.check and stale:
        return 1
    print(
        f"generate-dialect: OK ({len(enums)} enums, {len(messages)} messages)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

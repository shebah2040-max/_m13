// SPDX-License-Identifier: GPL-3.0-or-later
// AUTO-GENERATED from qgc/custom/mavlink/m130.xml by tools/generate-dialect.py.
// DO NOT EDIT. Re-run the generator when the dialect changes.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace m130::protocol::gen {

struct DialectEntry {
    std::uint32_t msg_id;
    const char*   name;
    std::size_t   wire_size;
    bool          inbound;
    float         rate_hz;
};

constexpr std::size_t kDialectMessageCount = 16;

extern const std::array<DialectEntry, kDialectMessageCount> kDialectTable;

/// Find a dialect entry by id. Returns nullptr if unknown.
const DialectEntry* findDialectEntry(std::uint32_t msg_id) noexcept;

} // namespace m130::protocol::gen

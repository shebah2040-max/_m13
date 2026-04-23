// SPDX-License-Identifier: GPL-3.0-or-later
//
// Aerospace Dark palette (MIL-STD-1472H / DO-178C colour semantics).
// Colour-blind-safe variants are selected via the `variant` property.
//
// variant values:
//   "default"     — standard aerospace dark
//   "deuter"      — Deuteranopia-safe
//   "protan"      — Protanopia-safe
//   "tritan"      — Tritanopia-safe
//   "achromat"    — Achromatopsia (greyscale + shape)

import QtQuick 2.15

QtObject {
    id: root

    property string variant: "default"

    // Semantic roles (do NOT hardcode these elsewhere — use QGCPalette).
    readonly property color primaryData      : "#ffffff"
    readonly property color commandedValue   : "#00d4ff"
    readonly property color nominal          : "#00b45a"
    readonly property color caution          : "#ffb000"
    readonly property color warning          : "#ff6000"
    readonly property color emergency        : "#c00000"

    // Surfaces.
    readonly property color background       : "#0d1117"
    readonly property color panel            : "#161b22"
    readonly property color divider          : "#30363d"

    function colorForLevel(level) {
        switch (level) {
        case 0:  return root.nominal
        case 1:  return root.primaryData
        case 2:  return root.caution
        case 3:  return root.warning
        case 4:  return root.emergency
        }
        return root.primaryData
    }

    // Palette remapping for colour-blind variants. The mapping prioritises
    // hue pairs (green/red) that remain distinguishable under each deficiency.
    readonly property var _remap: ({
        "deuter":  { nominal: "#009fcc", caution: "#ffb000", warning: "#d65f00", emergency: "#a30022" },
        "protan":  { nominal: "#0097c1", caution: "#f3b700", warning: "#e15c00", emergency: "#a50026" },
        "tritan":  { nominal: "#00a06a", caution: "#ea9e00", warning: "#c83f00", emergency: "#a50026" },
        "achromat":{ nominal: "#cccccc", caution: "#888888", warning: "#444444", emergency: "#000000" }
    })

    property color nominalVariant  : _remap[variant] ? _remap[variant].nominal   : nominal
    property color cautionVariant  : _remap[variant] ? _remap[variant].caution   : caution
    property color warningVariant  : _remap[variant] ? _remap[variant].warning   : warning
    property color emergencyVariant: _remap[variant] ? _remap[variant].emergency : emergency
}

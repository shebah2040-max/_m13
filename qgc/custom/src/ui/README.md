# `src/ui/` — Shared UI Components

| Component | Purpose | Requirements |
|---|---|---|
| `MasterCautionLight.qml` | ARINC 661 master lamp; embedded in every console header | UI-008, SAFE-004 |
| `AlertBanner.qml` | scrollable list of active alerts | SAFE-005/006 |
| `theme/QGCPaletteAerospace.qml` | semantic colour roles + 4 colour-blind variants | UI-002/003 |

All components:
- size exclusively via `ScreenTools.defaultFontPixelHeight/Width`
- colour exclusively via `QGCPalette` + `QGCPaletteAerospace`
- include shape-based cues so operators with colour vision deficiency still
  receive the signal (HAZ-009 mitigation).

QObject wrapping of `AlertManager` for QML binding lands in Pillar 3; until
then the components default to an empty state so the QGC build never fails
on missing context properties.

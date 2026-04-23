# `src/views/` — Multi-Console UI

> **Status**: Foundation scaffold. Detailed implementations migrate in per-view
> pillar PRs (Pillar 5 Operations/PreLaunch/RangeSafety; Pillar 7 Tuning/Analysis; Pillar 8 Admin).

Six specialised consoles replace the single FlyView. Each console is an
independent QML module wired into QGC via `CustomPlugin::createQmlApplicationEngine()`.

| Console | Role | Audience |
|---|---|---|
| `operations/` | primary flight display | Operator, Flight Director |
| `prelaunch/` | checklist + parameter snapshot | Operator |
| `rangesafety/` | IIP, corridor, FTS panel | RSO, Safety Officer |
| `tuning/` | MPC/MHE live tuning | Flight Director |
| `analysis/` | post-flight plots | Analyst |
| `admin/` | user mgmt + audit viewer | Admin |

Shared rules (also in `CODING_STYLE.md`):
- Sizes use `ScreenTools.defaultFontPixelHeight/Width` ONLY.
- Colors use `QGCPalette` ONLY (extended by `QGCPaletteAerospace` — see `src/ui/theme/`).
- The master caution/warning/emergency lamp from `src/ui/MasterCautionLight.qml`
  is embedded in every console header.
- RTL is handled by Qt's `LayoutMirroring` honouring `QLocale`.
- Multi-monitor layouts persist per user under `~/.config/M130/layouts/<user>.json`.

The current <ref_file file="/home/ubuntu/_m13/qgc/custom/src/FlyViewCustomLayer.qml" /> corresponds to the Operations Console and is preserved verbatim during Foundation; it will be split into the new layout in Pillar 5.

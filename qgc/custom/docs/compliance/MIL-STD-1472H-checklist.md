# MIL-STD-1472H — HMI Compliance Checklist

- **Document ID**: COMP-MIL1472H-001
- **Revision**: A

معيار الأميركي لـ Human Engineering — مرجعي للواجهات التشغيلية الحرجة.

## 5.1 General Design (ملخص)

| § | Requirement | Evidence in M130 GCS | Status |
|---|---|---|---|
| 5.1.1 | Dimensional compatibility | `src/views/` يستخدم `ScreenTools` النسبية | ✓ |
| 5.1.2 | Environment (lighting/vibration) | Dark theme + high-contrast theme | ✓ |
| 5.1.3 | Visibility & legibility | Font sizes نسبية، contrast ≥ 7:1 | ✓ |
| 5.1.4 | Standardization | `src/ui/theme/` موحّد | ✓ |
| 5.1.5 | Error prevention | Confirm dialogs + range checks | Partial |
| 5.1.6 | Automation limits | Manual override دائماً متاح | Pending |
| 5.1.7 | Controls reach | لوحات قابلة للتخصيص | ✓ |

## 5.2 Visual Displays

| § | Requirement | Evidence | Status |
|---|---|---|---|
| 5.2.1 | Brightness & contrast | Theme config + system brightness | ✓ |
| 5.2.2 | Glare/reflection | Dark theme افتراضي | ✓ |
| 5.2.3 | Color use | نظام ألوان DO-178C (white/cyan/green/amber/red) | ✓ |
| 5.2.4 | Colorblind accommodation | 4 palettes | Foundation |
| 5.2.5 | Symbology | رموز مع نصوص داعمة + اتساق | Pending |
| 5.2.6 | Alert/warning cues | Multi-modal (color + shape + sound) | Pending |
| 5.2.7 | Readability of text | Font size ≥ 10pt عند 1080p | ✓ |

## 5.3 Audio Signals

| § | Requirement | Evidence | Status |
|---|---|---|---|
| 5.3.1 | Audio warning separation | Advisory/Caution/Warning/Emergency بنغمات مميزة | Pending |
| 5.3.2 | Volume control | System audio + UI level | Pending |
| 5.3.3 | Voice messages | لاحقاً (Pillar Audio) | Planned |

## 5.4 Controls

| § | Requirement | Evidence | Status |
|---|---|---|---|
| 5.4.1 | Control type consistency | Buttons/toggles موحّدة عبر QGCPalette | ✓ |
| 5.4.2 | Spacing & size | `ScreenTools.defaultFontPixelWidth * 2` كحد أدنى | Pending |
| 5.4.3 | Guarded controls | FTS dual-auth + confirm dialog | Foundation |
| 5.4.4 | Discriminability | Shape + color + labels | Partial |

## 5.5 Human-Computer Dialog

| § | Requirement | Evidence | Status |
|---|---|---|---|
| 5.5.1 | Feedback (within 2s) | QML binding + status indicators | Pending |
| 5.5.2 | Error messages actionable | Alert context + recommended actions | Foundation |
| 5.5.3 | Undo/confirmation for destructive | Confirm dialogs implemented for Foundation | Partial |
| 5.5.4 | Cursor visibility | System cursor + QML focus | ✓ |
| 5.5.5 | Language support | AR + EN (RTL/LTR) | Foundation |

## 5.6 Workstation Design
خارج نطاق البرمجيات؛ توثيق في Operations.

## 5.7 Maintenance-Accessibility
لوحة Admin + Audit log + Configuration export/import.

## 5.8 Safety (HMI-related)
- HAZ-007 (alarm flood): mitigation عبر Alert priority + dedup
- HAZ-009 (colorblind): mitigation عبر 4 palettes + multi-modal cues

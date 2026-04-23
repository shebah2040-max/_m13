# ICD — Legacy DEBUG_FLOAT_ARRAY Mapping (Deprecated)

- **Document ID**: ICD-M130-DEBUGARRAY-001
- **Revision**: A
- **Status**: **DEPRECATED — للتوثيق فقط** (يُستخدم حتى اكتمال الهجرة إلى m130.xml)
- **Source of Truth**: `qgc/custom/src/telemetry/RocketTelemetryFactGroup.{h,cc}` (سيُستبدل)

## الرسالة

- **msgid**: `MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY` (350)
- **array_id**: `2`
- **name**: `"RktGNC"`
- **rate**: 20 Hz
- **source**: `AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_mpc/…/mavlink/streams/DEBUG_FLOAT_ARRAY.hpp`

## الخريطة الكاملة (58 حقلاً)

### P1 — Flight Critical (0-17)
| # | Fact | Units | Description |
|---|---|---|---|
| 0 | stage | enum | 0=IDLE, 1=BOOST, 2=CRUISE, 3=TERMINAL |
| 1 | tFlight | s | زمن الطيران |
| 2 | qDyn | Pa | الضغط الديناميكي |
| 3 | qKgf | kgf/m² | الضغط الديناميكي — وحدات بديلة |
| 4 | pitchAccelCmd | rad/s² | أمر تسارع الميل |
| 5 | yawAccelCmd | rad/s² | أمر تسارع الانحراف |
| 6 | blendAlpha | 0-1 | معامل خلط MHE↔EKF |
| 7 | deltaRoll | deg | أمر دحرجة |
| 8 | deltaPitch | deg | أمر ميل |
| 9 | deltaYaw | deg | أمر انحراف |
| 10-13 | fin1..4 | deg | انحراف الزعانف |
| 14 | altitude | m | ارتفاع |
| 15 | airspeed | m/s | سرعة هوائية |
| 16 | rho | kg/m³ | كثافة الهواء |
| 17 | servoOnlineMask | bitmask | 0b1111 = كل 4 online |

### P2 — Attitude & Position (18-32)
| # | Fact | Units |
|---|---|---|
| 18 | phi | deg |
| 19 | mpcSolveMs | ms |
| 20 | mheQuality | 0-1 |
| 21 | mpcSolveCount | — |
| 22 | theta | deg |
| 23 | psi | deg |
| 24 | alphaEst | deg |
| 25 | gammaRad | rad |
| 26 | posDownrange | m |
| 27 | posCrossrange | m |
| 28 | velDownrange | m/s |
| 29 | velDown | m/s |
| 30 | velCrossrange | m/s |
| 31 | bearingDeg | deg |
| 32 | targetRangeRem | m |

### P3 — Diagnostics & Timing (33-48)
| # | Fact | Units |
|---|---|---|
| 33 | launched | 0/1 |
| 34-36 | dtActual/Min/Max | ms |
| 37 | mheSolveMs | ms |
| 38 | mpcSolverStatus | enum |
| 39 | mpcSqpIter | — |
| 40 | mheValid | 0/1 |
| 41 | mheStatus | enum |
| 42 | xvalGammaErr | deg |
| 43 | xvalChiErr | deg |
| 44 | xvalAltErr | m |
| 45 | xvalPenalty | — |
| 46 | mheSolveUs | µs |
| 47 | mpcSolveUs | µs |
| 48 | cycleUs | µs |

### P4 — Event Counters & GPS (49-57)
| # | Fact | Units |
|---|---|---|
| 49 | mpcFailCount | — |
| 50 | mpcNanSkipCount | — |
| 51 | mheFailCount | — |
| 52 | finClampCount | — |
| 53 | xvalResetCount | — |
| 54 | servoOfflineEvt | — |
| 55 | gpsFixType | enum |
| 56 | gpsSatellites | — |
| 57 | gpsJammingState | enum |

## خطة الإلغاء

انظر `ICD-MAVLink.md` §6 للخطة الأربع مراحل.

# Flight Safety Envelope — M130 GCS

- **Document ID**: ENV-M130-GCS-001
- **Revision**: A (Foundation)
- **Source of Truth**: `src/safety/FlightSafetyEnvelope.cc` defaults

## 1. مبدأ العمل

"Safety Envelope" هي حدود رسمية لكل متغيّر مرصود. خرق أي حد يُفعّل تنبيهاً بمستوى محدد وفق شدة الخرق.

## 2. بنية الحد

لكل متغير:
- **Nominal Range** [lo_ok, hi_ok] — لا تنبيه
- **Advisory Range** [lo_adv, hi_adv] — Advisory (معلوماتي)
- **Caution Range** [lo_caut, hi_caut] — Caution (مراقبة)
- **Warning Range** [lo_warn, hi_warn] — Warning (تحرك)
- **Emergency** خارج هذا — Emergency (إجراء فوري)

## 3. Envelope الأساس (Foundation)

> هذه قيم افتراضية؛ ستُعدّل لكل بيئة اختبار/إطلاق عبر ملف تكوين موقَّع.

### 3.1 Attitude & Angular Rates

| Variable | Unit | Nominal | Advisory | Caution | Warning | Emergency |
|---|---|---|---|---|---|---|
| phi (roll) | deg | [-10, 10] | [-20, 20] | [-45, 45] | [-90, 90] | outside |
| theta (pitch) | deg | [75, 90] if BOOST | [70, 90] | [60, 90] | [45, 90] | <45 |
| psi rate | deg/s | [-5, 5] | [-15, 15] | [-30, 30] | [-60, 60] | outside |
| alpha_est | deg | [-3, 3] | [-5, 5] | [-8, 8] | [-12, 12] | outside |

### 3.2 Translational

| Variable | Unit | Nominal | Advisory | Caution | Warning | Emergency |
|---|---|---|---|---|---|---|
| altitude | m | configured | configured | configured | configured | configured |
| airspeed | m/s | [0, 800] | [0, 900] | [0, 1000] | [0, 1100] | >1100 |
| pos_crossrange | m | [-200, 200] | [-500, 500] | [-1000, 1000] | [-2000, 2000] | outside |
| q_dyn | Pa | [0, 60000] | [0, 80000] | [0, 100000] | [0, 120000] | >120000 |

### 3.3 Control & Solver Health

| Variable | Unit | Nominal | Advisory | Caution | Warning | Emergency |
|---|---|---|---|---|---|---|
| fin deflection | deg | [-10, 10] | [-15, 15] | [-20, 20] | [-25, 25] | outside |
| mpc_solve_us | µs | [0, 3000] | [0, 5000] | [0, 8000] | [0, 15000] | >15000 |
| mpc_fail_count | delta/s | 0 | 1 | 3 | 5 | >5 |
| mhe_quality | ratio | [0.8, 1.0] | [0.6, 1.0] | [0.4, 1.0] | [0.2, 1.0] | <0.2 |
| blend_alpha | ratio | [0.0, 1.0] | — | — | — | outside |

### 3.4 Communication & System Health

| Variable | Unit | Nominal | Advisory | Caution | Warning | Emergency |
|---|---|---|---|---|---|---|
| heartbeat_age | s | [0, 0.5] | [0, 1.0] | [0, 3.0] | [0, 10.0] | >10 (BOOST/CRUISE) |
| gps_fix_type | enum | 3D+ | 3D+ | 2D+ | any | none |
| gps_satellites | count | ≥10 | ≥8 | ≥6 | ≥4 | <4 |
| gps_jamming | level | 0-1 | 0-2 | 0-3 | 0-4 | >4 |
| servo_online_mask | bitmask | 0b1111 | 0b1110/1101/1011/0111 | 0b11xx | 0b1xxx | 0 |

## 4. Context-Dependent Envelopes

تختلف الحدود وفق **FlightPhase**:
- **IDLE/PRELAUNCH/ARMED**: حدود صارمة للأمان (لا حركة)
- **BOOST**: أوسع حدود للتسارع
- **CRUISE**: حدود أضيق للاستقرار
- **TERMINAL**: حدود مركّزة على الدقة النهائية
- **ABORT**: يُتجاوز جميع الفحوصات باستثناء السلامة الأرضية

## 5. آلية التكوين

Envelope يُقرأ من `qgc/custom/config/safety-envelope.yaml` الموقَّع:
```yaml
envelope:
  schema_version: 1
  mission: "M130-TestA"
  phase: BOOST
  variables:
    theta_deg:
      nominal: [75, 90]
      advisory: [70, 90]
      caution: [60, 90]
      warning: [45, 90]
```

ملف التكوين يُوقَّع بمفتاح Safety Officer قبل استخدامه.

## 6. اختبار

كل حد يختبر في `tests/unit/FlightSafetyEnvelopeTest.cpp`:
- قيمة عند الحد → التصنيف الأدنى
- قيمة داخل نطاق → لا تنبيه
- قيمة خارج نطاق Emergency → Emergency

## 7. التوثيق
- أي تغيير في Envelope يتطلب موافقة Safety Officer
- كل نسخة envelope تُحفَظ في `docs/operations/envelopes/` مع توقيعها

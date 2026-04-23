# SRS — M130 GCS Software Requirements Specification

- **Document ID**: SRS-M130GCS-001
- **Revision**: A (Foundation)
- **Standard**: ISO/IEC/IEEE 29148:2018
- **Scope**: محطة التحكم الأرضية لصاروخ M130 (مبنية على فورك QGroundControl)
- **Target Criticality**: DO-178C DAL-C (عرض معلومات أرضي داعم لطيران)

---

## 1. Introduction

### 1.1 Purpose
هذه الوثيقة تحدد متطلبات **محطة M130 GCS** — محطة تحكم أرضية متخصصة لصاروخ M130 مبنية على QGroundControl. تستقبل تيليمتري GNC عبر MAVLink، تعرض الحالة، تدير الأوامر، وتسجّل البيانات للتحليل والاعتماد.

### 1.2 Scope
المحطة:
- تعرض بيانات المتحكم/المقدّر (MPC/MHE) في الزمن الحقيقي
- توفر عدة كونسولات متخصصة (Operations, Pre-Launch, Range Safety, Tuning, Analysis, Admin)
- تُنفذ أوامر مصادَقة مع سلطة أمان طيرانية
- تُسجّل جميع البيانات والأحداث مع chain-of-custody
- تتكامل مع SITL/HIL للاختبار

### 1.3 Definitions
| مصطلح | تعريف |
|---|---|
| GCS | Ground Control Station |
| GNC | Guidance, Navigation, and Control |
| MPC | Model Predictive Controller |
| MHE | Moving Horizon Estimator |
| FTS | Flight Termination System |
| RSO | Range Safety Officer |
| FDR | Flight Data Recorder |
| IIP | Instantaneous Impact Point |
| DAL | Design Assurance Level |

### 1.4 References
- ISO/IEC/IEEE 29148:2018
- ISO/IEC/IEEE 12207:2017
- DO-178C / ED-12C
- DO-278A / ED-109A
- MIL-STD-1472H
- MIL-STD-882E
- ISO/IEC 27001:2022
- NIST SP 800-53 Rev. 5
- `design/SystemArchitecture.md`
- `safety/HazardLog.md`

---

## 2. Overall Description

### 2.1 Product Perspective
المحطة جزء من نظام M130 الأوسع:
- **Flight Segment**: صاروخ M130 مع PX4/Android autopilot + وحدة `rocket_mpc`
- **Communication**: MAVLink 2 موقّع + قناة USB/Serial/UDP
- **Ground Segment**: محطة M130 GCS (هذه) + بنية تحتية للتسجيل + خدمات خارجية (طقس، NOTAMs)

### 2.2 User Classes
| المستخدم | المسؤولية | الصلاحيات |
|---|---|---|
| Observer | مراقبة فقط | READ |
| Operator | تشغيل عادي | READ + COMMAND_BENIGN |
| Flight Director | قيادة المهمة | + COMMAND_ELEVATED |
| Safety Officer | صلاحيات أمان | + ABORT + HOLD |
| Range Safety Officer (RSO) | تنفيذ الإنهاء | + FTS (مع dual-auth) |
| Admin | إدارة النظام | الكل + User Management |

### 2.3 Operating Environment
- Qt 6.5+ على Linux/Windows/macOS/Android
- شاشات 13"–65"، resolution ≥ 1920×1080
- CPU ≥ 4 cores @ 2.0 GHz، RAM ≥ 8 GB
- USB/Serial/UDP/TCP للاتصال
- تخزين دائم ≥ 500 GB للـ FDR

### 2.4 Constraints
- C++20 + Qt 6 + QML
- التوافق مع MAVLink 2 dialect القياسي + dialect `m130` مخصص
- الأداء: تحديث الواجهة ≥ 20 Hz، latency ≤ 100 ms بعد وصول الرسالة
- الذاكرة: ≤ 1 GB RAM أثناء العمل العادي
- لغة العرض: عربي + إنكليزي (RTL/LTR صحيحان)

---

## 3. Functional Requirements

> **الترميز**: `REQ-M130-GCS-<الفئة>-<الرقم>`
> الفئات: `PROT` (البروتوكول)، `TELE` (التيليمتري)، `SAFE` (السلامة)، `CMD` (الأوامر)، `LOG` (التسجيل)، `UI` (الواجهة)، `SEC` (الأمن)، `ACC` (الصلاحيات)، `QA` (ضمان الجودة)

### 3.1 Protocol Requirements

- **REQ-M130-GCS-PROT-001**: يجب دعم MAVLink 2 signing على كل القنوات.
- **REQ-M130-GCS-PROT-002**: يجب استخدام dialect `m130.xml` الرسمي؛ يحظر استخدام `DEBUG_*` لبيانات إنتاجية.
- **REQ-M130-GCS-PROT-003**: يجب رفض الرسائل إذا كان `protocol_version` في `M130_HEARTBEAT_EXTENDED` غير متوافق.
- **REQ-M130-GCS-PROT-004**: يجب تسجيل كل رسالة واردة/صادرة في FDR قبل المعالجة.
- **REQ-M130-GCS-PROT-005**: يجب رفع Alert Caution إذا فُقد heartbeat > 1 s.
- **REQ-M130-GCS-PROT-006**: يجب رفع Alert Warning إذا فُقد heartbeat > 3 s.
- **REQ-M130-GCS-PROT-007**: يجب رفع Alert Emergency إذا فُقد heartbeat > 10 s والمهمة في BOOST/CRUISE.

### 3.2 Telemetry Requirements

- **REQ-M130-GCS-TELE-001**: يجب عرض جميع الحقول الـ 58 (الحد الأدنى) من تيليمتري GNC عبر Fact System.
- **REQ-M130-GCS-TELE-002**: يجب أن تكون كل Fact مطابقة لتعريف `m130.xml` بالاسم والوحدة.
- **REQ-M130-GCS-TELE-003**: يجب رفع Alert إذا تخطت أي Fact حدود `SafetyEnvelope.md`.
- **REQ-M130-GCS-TELE-004**: يجب تسجيل staleness (الزمن منذ آخر تحديث) لكل Fact؛ تحديث > 500 ms = Caution، > 2 s = Warning.
- **REQ-M130-GCS-TELE-005**: يجب دعم quad-redundant cross-check بين MHE/EKF عندما يتوفران.

### 3.3 Safety Requirements

- **REQ-M130-GCS-SAFE-001**: يجب تنفيذ State Machine مهمة رسمية: `UNKNOWN → IDLE → PRELAUNCH → ARMED → BOOST → CRUISE → TERMINAL → LANDED` مع `ABORT` من أي حالة.
- **REQ-M130-GCS-SAFE-002**: يجب أن يقبل الانتقال بين الحالات شروطاً محدّدة فقط (guard conditions) مع تسجيل رفض أي انتقال غير صالح.
- **REQ-M130-GCS-SAFE-003**: يجب أن تعمل Watchdogs مستقلة على: link-loss، data-staleness، solver-failure، sensor-invalid، power-anomaly.
- **REQ-M130-GCS-SAFE-004**: يجب عرض Master Caution + Master Warning + Master Emergency بوضوح على كل كونسول.
- **REQ-M130-GCS-SAFE-005**: يجب أن يكون نظام التنبيه مطابقاً لـ ARINC 661 (4 مستويات: Advisory/Caution/Warning/Emergency).
- **REQ-M130-GCS-SAFE-006**: يجب تسجيل كل alert في AuditLog مع صلاحيات المستخدم الذي صادق عليه.
- **REQ-M130-GCS-SAFE-007**: يجب حساب وعرض Instantaneous Impact Point (IIP) في الزمن الحقيقي.
- **REQ-M130-GCS-SAFE-008**: يجب تصور Flight Corridor/Safety Footprint على خريطة RSO.
- **REQ-M130-GCS-SAFE-009**: يجب أن يتطلب Flight Termination Command **مصادقة ثنائية** (RSO + Safety Officer) مع confirm dialog ومهلة.
- **REQ-M130-GCS-SAFE-010**: يجب رفض FTS من أي مستخدم غير RSO.

### 3.4 Command Requirements

- **REQ-M130-GCS-CMD-001**: يجب أن ترفض كل الأوامر إذا لم يكن المستخدم ممنوحاً الصلاحية المناسبة (مصفوفة RBAC).
- **REQ-M130-GCS-CMD-002**: يجب أن يعرض كل أمر حوار تأكيد قبل الإرسال (ما عدا الأوامر ذات المعدل العالي كـ manual override).
- **REQ-M130-GCS-CMD-003**: يجب أن ينتظر كل أمر ACK من الطائرة مع مهلة افتراضية 2 s وإعادة محاولة واحدة.
- **REQ-M130-GCS-CMD-004**: يجب التحقق من النطاق، ومعدل التغير، والتوقيت قبل إرسال أي أمر.
- **REQ-M130-GCS-CMD-005**: يجب تسجيل كل أمر في AuditLog (مُصدِر، توقيت، نطاقات، نتيجة).

### 3.5 Logging Requirements

- **REQ-M130-GCS-LOG-001**: يجب تسجيل كل الرسائل الواردة والصادرة في Flight Data Recorder (FDR).
- **REQ-M130-GCS-LOG-002**: يجب أن يكون FDR متعدد التنسيق: خام MAVLink + Parquet + JSON events.
- **REQ-M130-GCS-LOG-003**: يجب توقيع كل ملف FDR بـ HMAC-SHA256 مع مفتاح يُدار داخلياً.
- **REQ-M130-GCS-LOG-004**: يجب أن يكون AuditLog **append-only** مع توقيع متسلسل (hash chain).
- **REQ-M130-GCS-LOG-005**: يجب إنشاء تقرير PDF آلي بعد كل مهمة من FDR.
- **REQ-M130-GCS-LOG-006**: يجب حفظ اللقطات البصرية (screenshots) كل 10 ثوانٍ خلال المهمة.
- **REQ-M130-GCS-LOG-007**: يجب دعم Replay Engine بسرعة 0.1× إلى 100×.

### 3.6 UI Requirements

- **REQ-M130-GCS-UI-001**: يجب توفير كونسولات متعددة قابلة للتبديل: Operations, Pre-Launch, Range Safety, Tuning, Analysis, Admin.
- **REQ-M130-GCS-UI-002**: يجب أن تتبع الواجهة نظام ألوان DO-178C / MIL-STD-1472H (Primary=white, Commanded=cyan, Nominal=green, Caution=amber, Warning=red).
- **REQ-M130-GCS-UI-003**: يجب دعم 4 أوضاع لعمى الألوان (Deuteranopia/Protanopia/Tritanopia/Achromatopsia).
- **REQ-M130-GCS-UI-004**: يجب استخدام `ScreenTools.defaultFontPixelHeight/Width` فقط لأحجام العناصر.
- **REQ-M130-GCS-UI-005**: يجب استخدام `QGCPalette` فقط للألوان؛ لا ألوان hardcoded خارج تعريفات الـ palette.
- **REQ-M130-GCS-UI-006**: يجب توفير RTL صحيح للعربية واستجابة تخطيط للشاشات 13"–65".
- **REQ-M130-GCS-UI-007**: يجب أن تحفظ layouts الكونسولات لكل مستخدم مع دعم multi-monitor.
- **REQ-M130-GCS-UI-008**: يجب أن تعرض الحالة master caution/warning/emergency في شريط ثابت في كل الكونسولات.

### 3.7 Security Requirements

- **REQ-M130-GCS-SEC-001**: يجب مصادقة المستخدم محلياً بـ argon2id مع تكلفة زمنية ≥ 100 ms.
- **REQ-M130-GCS-SEC-002**: يجب دعم LDAP/AD كمصدر مصادقة اختياري.
- **REQ-M130-GCS-SEC-003**: يجب دعم TOTP (RFC 6238) لعمليات FTS وتغيير الصلاحيات.
- **REQ-M130-GCS-SEC-004**: يجب أن تكون جلسة العمل محدودة (افتراضي 8 ساعات، قابل للتخصيص لكل دور).
- **REQ-M130-GCS-SEC-005**: يجب أن تطلب إعادة مصادقة قبل أي عملية حرجة (FTS, user admin).
- **REQ-M130-GCS-SEC-006**: يجب تشفير قناة GCS↔Relay بـ TLS 1.3.
- **REQ-M130-GCS-SEC-007**: يجب أن تُوقَّع جميع رسائل MAVLink باستخدام MAVLink 2 signing.
- **REQ-M130-GCS-SEC-008**: يجب تدوير مفاتيح التوقيع بشكل دوري (افتراضي 30 يوماً).
- **REQ-M130-GCS-SEC-009**: يجب حماية المفاتيح باستخدام OS keychain أو HSM.

### 3.8 Access Control Requirements

- **REQ-M130-GCS-ACC-001**: يجب تطبيق RBAC مع الأدوار المعرّفة في 2.2.
- **REQ-M130-GCS-ACC-002**: يجب تسجيل كل تغيير في الصلاحيات في AuditLog.
- **REQ-M130-GCS-ACC-003**: يجب عدم السماح للمستخدم بتعديل صلاحياته الخاصة.
- **REQ-M130-GCS-ACC-004**: يجب قفل حساب المستخدم بعد 5 محاولات فاشلة متتالية.

### 3.9 Quality Assurance Requirements

- **REQ-M130-GCS-QA-001**: يجب أن يكون code coverage ≥ 80% statement + branch على الكود الأمني والمتحكمات.
- **REQ-M130-GCS-QA-002**: يجب MC/DC coverage على Safety Kernel (DO-178C DAL-C).
- **REQ-M130-GCS-QA-003**: يجب أن تجتاز جميع PRs: clang-format + clang-tidy + clazy + pyright + ruff + shellcheck.
- **REQ-M130-GCS-QA-004**: يجب أن تُولَّد مصفوفة التتبع تلقائياً ويتحقق منها CI.
- **REQ-M130-GCS-QA-005**: يجب أن لا يُدمج PR بدون مراجعة من مهندس مستقل.

---

## 4. Non-Functional Requirements

- **NFR-P-001**: زمن استجابة الواجهة ≤ 100 ms بعد وصول الرسالة.
- **NFR-P-002**: استهلاك RAM ≤ 1 GB في العمل العادي، ≤ 2 GB أثناء replay.
- **NFR-P-003**: بدء تشغيل المحطة ≤ 10 s.
- **NFR-R-001**: MTBF ≥ 5000 ساعة تشغيل بدون crash.
- **NFR-R-002**: استرداد تلقائي من link-loss بدون فقدان بيانات FDR.
- **NFR-U-001**: قابلية استخدام — مشغل مدرَّب ينجز checklist ما قبل الإطلاق في ≤ 15 دقيقة.
- **NFR-M-001**: قابلية الصيانة — إضافة حقل تيليمتري جديد يتطلب تحديث `m130.xml` فقط + ملف factgroup واحد.
- **NFR-M-002**: قابلية الاختبار — كل وحدة قابلة للاختبار باستقلال عن Qt GUI.
- **NFR-C-001**: التوافق مع Qt 6.5 LTS وما فوق.
- **NFR-C-002**: التوافق مع PX4 1.14 وما فوق.

---

## 5. Verification Matrix (Summary)

الجدول الكامل في `traceability.csv`. كل REQ يجب أن يرتبط بـ:
- تصميم في `design/`
- كود في `src/`
- اختبار واحد على الأقل في `tests/`
- طريقة تحقق: Inspection / Analysis / Demonstration / Test

---

## 6. Change Log

| Rev | Date | Author | Description |
|---|---|---|---|
| A  | 2026-04-23 | Foundation Team | النسخة الأولى — Foundation PR |

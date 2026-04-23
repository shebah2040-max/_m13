# System Architecture — M130 GCS

- **Document ID**: DESIGN-M130-GCS-001
- **Revision**: A (Foundation)

## 1. Overview

M130 GCS هي تخصيص لـ QGroundControl يضيف 7 طبقات:

```
┌────────────────────────────────────────────────────────────────┐
│                          VIEWS LAYER                           │
│  Operations │ PreLaunch │ Range Safety │ Tuning │ Analysis │Admin│
└──────┬────────────────┬──────────────┬─────────┬────────┬───────┘
       │                │              │         │        │
       ▼                ▼              ▼         ▼        ▼
┌────────────────────────────────────────────────────────────────┐
│                      UI / THEME LAYER                          │
│   QGCPaletteAerospace │ AlertBanner │ MasterCautionLight       │
└────────────────────────┬───────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────────────────┐
│                       SAFETY KERNEL                            │
│   MissionStateMachine │ AlertManager │ FlightSafetyEnvelope    │
│   Watchdog │ CommandAuthorization │ FlightTerminationService   │
└────────────────────────┬───────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────────────────┐
│                       ACCESS LAYER                             │
│        UserManager │ Role │ SessionManager                     │
└────────────────────────┬───────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────────────────┐
│                     TELEMETRY LAYER                            │
│  GncStateFactGroup │ MheFactGroup │ MpcFactGroup │ EventCounters│
│              RocketTelemetryFactGroup (legacy)                 │
└────────────────────────┬───────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────────────────┐
│                     PROTOCOL LAYER                             │
│  M130Dialect │ MessageRouter │ ProtocolVersion │ MAVLink 2     │
└────────────────────────┬───────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────────────────┐
│                     LOGGING LAYER                              │
│  FlightDataRecorder │ AuditLogger │ ChainOfCustody              │
└────────────────────────────────────────────────────────────────┘
```

## 2. Layer Responsibilities

### 2.1 Protocol Layer
- إدارة اتصالات MAVLink 2 مع التوقيع
- فك وتجميع الرسائل من dialect `m130.xml`
- التحقق من protocol_version في heartbeat
- التوجيه إلى Telemetry Layer

### 2.2 Telemetry Layer
- استقبال الرسائل من Protocol وتحديث FactGroups
- تتبع staleness لكل Fact
- تنبيه Safety Kernel عند خرق حدود الـ envelope

### 2.3 Safety Kernel
- **MissionStateMachine**: حارس حالة المهمة مع انتقالات مشروطة
- **AlertManager**: نظام تنبيهات ذو 4 مستويات (ARINC 661)
- **Watchdog**: مراقبة link/data/solver/sensor/power
- **FlightSafetyEnvelope**: فحص القيم ضد حدود آمنة
- **CommandAuthorization**: مصفوفة RBAC للأوامر
- **FlightTerminationService**: FTS مع dual authorization

### 2.4 Access Layer
- **UserManager**: إدارة مستخدمين محليين (argon2id) + دمج LDAP/AD اختياري
- **Role**: 6 أدوار معرّفة
- **SessionManager**: جلسات محدودة الزمن مع reauth للعمليات الحرجة

### 2.5 Logging Layer
- **FlightDataRecorder**: تسجيل متعدد التنسيق (raw MAVLink + Parquet + JSON events)
- **AuditLogger**: append-only hash-chained audit log
- **ChainOfCustody**: توقيع HMAC-SHA256 للملفات مع تدوير المفاتيح

### 2.6 UI / Theme Layer
- **QGCPaletteAerospace**: 4 بليتات (Default + 3 colorblind + Achromatopsia)
- **AlertBanner**: عرض التنبيهات النشطة
- **MasterCautionLight**: مؤشرات ثابتة في كل كونسول

### 2.7 Views Layer
- 6 كونسولات متخصصة قابلة للتبديل
- Layouts محفوظة لكل مستخدم
- دعم multi-monitor

## 3. Data Flow (مثال: رسالة واردة)

1. MAVLink 2 frame → **Protocol Layer** يتحقق من التوقيع و sequence
2. Protocol Layer يستدعي **FlightDataRecorder.appendRaw()** قبل المعالجة
3. Protocol Layer يفك الرسالة ويستدعي **MessageRouter.route()**
4. MessageRouter يحدد الـ FactGroup المستهدف ويستدعي `handleMessage()`
5. FactGroup يحدث قيم Facts وينشر signal
6. **Watchdog** يرصد التحديث ويعيد ضبط staleness timer
7. **FlightSafetyEnvelope** يتحقق من القيم
8. عند خرق → **AlertManager.raise(level, id, context)**
9. **MasterCautionLight** + **AlertBanner** تُحدَّث تلقائياً
10. مشغل يُصادق على التنبيه → **AuditLogger.log()**

## 4. Data Flow (مثال: أمر صادر)

1. مشغل يضغط على زر FTS في Range Safety Console
2. Console يعرض confirm dialog مع dual-auth prompt
3. RSO يدخل TOTP ← يُرسل إلى **UserManager.verifyTotp()**
4. Safety Officer يدخل TOTP ← تحقق ثانٍ
5. Console يبني الأمر عبر **CommandAuthorization.buildCommand(FTS, args)**
6. **CommandAuthorization** يتحقق من الصلاحيات والحدود
7. إن وافق → يُنشئ `M130_COMMAND_FTS` مع tokens
8. **AuditLogger.log()** قبل الإرسال
9. **Protocol Layer** يرسل ويُدخِل timeout 500 ms
10. في انتظار ACK → إن لم يصل → إعادة محاولة 3 مرات
11. عند ACK → **AlertManager.raise(Emergency, "FTS_ARMED_CONFIRMED")**

## 5. Threading & Concurrency

- **Main UI thread**: Qt event loop، QML، user interactions
- **Protocol thread**: MAVLink I/O (اعتمادًا على QGC)
- **Logging thread**: تسجيل متزامن مع throttling للـ I/O
- **Watchdog thread** (أو QTimer في main): فحص staleness
- كل وحدة آمنة-سلسلة (thread-safe) عبر `QMutex` أو signal/slot queued

## 6. Error Handling Strategy

| نوع الخطأ | الاستراتيجية |
|---|---|
| رسالة MAVLink فاسدة | تسجيل + تخطي (لا crash) |
| فشل solver | Alert Warning + عد العدادات |
| فقدان link | Watchdog → Alert (Caution/Warning/Emergency) |
| استثناء C++ غير معالج | `qFatal` مع تسجيل core dump |
| فشل مصادقة | Alert Advisory + lockout بعد 5 محاولات |
| فشل توقيع | رفض الرسالة + Alert Warning |

## 7. Build & Deploy

- CMake كما هو في QGC الأصلي
- مجلد `qgc/custom/` هو الـ plugin root
- تبنى مع `QGC_CUSTOM_BUILD` معرَّف
- الاختبارات منفصلة في `qgc/custom/tests/` مع CTest labels `M130` + `Unit`/`Integration`

## 8. Security Model

- **Threat Model**: في `compliance/NIST-800-53-mapping.md` §3
- **Key Management**: مفاتيح HMAC تُخزن في OS keychain أو HSM (اختياري)
- **MAVLink signing**: مفتاح يُولَّد عند كل flight_id ويُنقل بشكل out-of-band
- **TLS 1.3**: مُلزم على relay channels

## 9. Testability

كل طبقة قابلة للاختبار باستقلال:
- **Safety Kernel**: pure C++ (بلا Qt GUI) — unit tests مباشرة
- **Telemetry FactGroups**: Qt Test + QSignalSpy
- **Views QML**: Qt Quick Test
- **Integration**: SITL bridge integration tests

## 10. Future Extensions (مخطط)

- **IRIG-106 export** للتكامل مع أنظمة range tracking
- **ARINC 664 AFDX** للـ ground networks الحرجة
- **OpenTelemetry** للـ observability

## ملاحق (Anchors)

- `#protocol` — Protocol Layer details
- `#telemetry` — Telemetry Layer details
- `#safety` — Safety Kernel details
- `#access` — Access Layer details
- `#logging` — Logging Layer details
- `#ui` — UI/Theme Layer details
- `#security` — Security architecture

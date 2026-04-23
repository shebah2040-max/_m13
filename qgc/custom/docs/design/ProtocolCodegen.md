---
noteId: "2e8723d03f2e11f1be0e05e75d28fafd"
tags: []

---

# Dialect Code Generation (Pillar 2)

- **Document ID** : DESIGN-M130-PROTO-CODEGEN-001
- **Revision**    : A (initial, Pillar 2)
- **Scope**       : توليد C++ من `qgc/custom/mavlink/m130.xml` وهجرة التيليمتري من `DEBUG_FLOAT_ARRAY` إلى الرسائل الرسمية.
- **Authoritative sources** :
  - `qgc/custom/mavlink/m130.xml` (ICD)
  - `qgc/custom/tools/generate-dialect.py` (generator)
  - `qgc/custom/src/protocol/generated/*.generated.{h,cc}` (artifacts)

---

## 1. لماذا مولّد مخصص ولم نعتمد `pymavlink`؟

- نريد بقاء الـ CI **بلا تبعية Python خارجية** — الاختبارات تعمل على stdlib فقط (ubuntu 22.04 default).
- نحتاج طبقة رقيقة: `struct` لكل رسالة + `pack/unpack` على payload بدون إطار الـ MAVLink الكامل (الذي يتولاه QGC stack).
- توليد enums بصيغة `enum class` مطابقة للـ Safety Kernel بدل `#define`s.
- السيطرة على `operator==` + `kWireSize` + `kName` لكل رسالة لخدمة اختبارات الـ roundtrip والـ FDR.

## 2. سياسة التوليد

- **المصدر الوحيد للحقيقة** هو `m130.xml`. أي تعديل يدوي على ملفات `*.generated.*` يرفضه CI (`generate-dialect.py --check`).
- **الـ diff يُراجَع يدوياً** ويُدخل ضمن الـ PR — لا نعتمد على توليد وقت البناء لتقليل تبعيات CMake.
- الترتيب على السلك (wire order) = ترتيب MAVLink v2 (فرز مستقر حسب حجم النوع تنازلياً). لا نستخدم CRC-extra على هذه الطبقة لأن QGC يتحقق منها قبل استدعاء `mavlinkMessage`.

## 3. المخرجات

| ملف | الوصف |
|-----|------|
| `M130Enums.generated.h` | enum classes مع `toString()` |
| `M130Messages.generated.h` | structs لكل رسالة مع `kMsgId/kWireSize/kName` و`pack/unpack/operator==` |
| `M130Messages.generated.cc` | تنفيذ `pack/unpack/operator==` (memcpy + wire-order) |
| `M130DialectTable.generated.h/cc` | جدول `{id, name, wire_size, inbound, rate_hz}` + `findDialectEntry()` |

## 4. Runtime integration

```
MAVLink frame  ───▶  QGC stack (CRC check, v2 framing)
                         │
                         ▼
         CustomPlugin::mavlinkMessage(vehicle, link, msg)
                         │
              ┌──────────┴───────────┐
              ▼                      ▼
  isM130Id(msg.msgid)?    msg.msgid == DEBUG_FLOAT_ARRAY?
       │                           │
       ▼                           ▼
_dispatchM130Message()     RocketTelemetryFactGroup (legacy fallback)
       │
       ├─ M130_HEARTBEAT_EXTENDED → feed watchdog + checkCompat()
       ├─ M130_GNC_STATE          → M130GncStateFactGroup.applyState()
       │                           + SafetyKernel.evaluateSample(phi, alpha_est)
       └─ … other ids → no-op (tracked in ICD §3.2)
```

## 5. طريق الهجرة

1. **اليوم**: الـ firmware يبثّ `DEBUG_FLOAT_ARRAY` حصراً. `RocketTelemetryFactGroup` هو الذي يُملأ. `M130GncStateFactGroup` موجود لكنه فارغ.
2. **المرحلة التالية (خارج هذا الـ PR)**: تحديث `rocket_mpc` في PX4 ليبثّ `M130_GNC_STATE` + `M130_HEARTBEAT_EXTENDED`. عندها الـ FactGroup الجديد يُملأ تلقائياً.
3. **نهاية الهجرة**: إزالة `RocketTelemetryFactGroup` من `CustomFirmwarePlugin::factGroups()` + تحديث `FlyViewCustomLayer.qml` ليستخدم `vehicle.getFactGroup("m130Gnc")`.

## 6. الاختبارات (`tests/core/M130GeneratedCodecTest.cc`)

| الاختبار | يغطي |
|---------|------|
| `dialectTableIsPopulated` | الجدول يضم كل الـ 16 رسالة + بحث بالـ id |
| `gncStateRoundTrip` | pack → unpack يسترجع كل الحقول العددية |
| `commandFtsRoundTripWithArrays` | `uint8_t[32]` tokens يجتازان الـ roundtrip |
| `unpackRejectsShortPayload` | رفض أي payload أقصر من `kWireSize` |
| `packRejectsShortBuffer` | رفض buffers صغيرة بدل الكتابة خارج الحدود |
| `wireSizesMatchXmlSums` | `kWireSize` يطابق مجموع أحجام الحقول |
| `enumToStringCoversAllLevels` | `toString()` على كل قيم `M130AlertLevel` و`M130FlightPhase` |

## 7. المفترضات (Assumptions)

- **Endianness**: المنصات المستهدفة (x86-64, ARM64) little-endian. `memcpy` كافٍ بلا swap.
- **Alignment**: الـ structs تستخدم ترتيب الإعلان (declaration order) وليس wire order، لذا لا قيود aggregate على `pack()/unpack()`.
- **Extensions**: الـ XML الحالي لا يستخدم `extensions` tag؛ عند إضافتها نُحدّث المولّد لإلحاقها في النهاية كما تفعل MAVLink v2.

## 8. القيود المعروفة (Known Limitations)

- لا توليد `encode()` على مستوى إطار MAVLink v2 — هذه الطبقة تُفوَّض لـ QGC stack (`mavlink_msg_…_encode`). توليد مكتمل يحتاج CRC-extra لكل رسالة ويمكن إضافته في PR لاحق تحت Pillar 2.1.
- لا توليد Python — `m13/` يبقى على بنيته الحالية.
- `qobject_cast<CustomFirmwarePlugin*>` في `_dispatchM130Message` يفترض PX4 حصراً وهو صحيح (APM معطّل في `CustomOverrides.cmake`).

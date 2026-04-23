# M130 GCS — Compliance & Engineering Documentation

> هذا المجلد هو **المصدر المعياري الوحيد** لوثائق محطة M130 GCS الأرضية.
> جميع التغييرات على الكود يجب أن تُعكس هنا وفق مصفوفة التتبع.

## الهيكل

```
docs/
├── README.md                   ← هذا الملف (خريطة التوثيق)
├── requirements/
│   ├── SRS-M130GCS.md          ← مواصفات المتطلبات البرمجية (ISO/IEC/IEEE 29148)
│   ├── ICD-MAVLink.md          ← وثيقة التحكم في الواجهة للـ MAVLink
│   ├── ICD-DebugFloatArray-legacy.md  ← خريطة DEBUG_FLOAT_ARRAY الحالية (مُهجّرة)
│   └── traceability.csv        ← Req ID ↔ Design ↔ Code ↔ Tests
├── design/
│   └── SystemArchitecture.md   ← المعمارية العامة + المخططات
├── plans/
│   ├── SDP.md                  ← خطة تطوير البرمجيات
│   ├── SVP.md                  ← خطة التحقق (Verification)
│   ├── SCMP.md                 ← خطة إدارة التكوين (Configuration Mgmt)
│   └── SQAP.md                 ← خطة ضمان الجودة (QA)
├── safety/
│   ├── HazardLog.md            ← سجل الأخطار (MIL-STD-882E)
│   ├── FMEA.md                 ← تحليل أنماط الفشل والآثار
│   └── SafetyEnvelope.md       ← علبة الأمان الطيرانية
├── compliance/
│   ├── ISO9001-checklist.md    ← قائمة مطابقة ISO 9001:2015
│   ├── AS9100D-delta.md        ← الفروق بين ISO 9001 و AS9100D
│   ├── DO-178C-objectives-DAL-C.md  ← أهداف DO-178C (DAL-C)
│   ├── DO-278A-objectives.md   ← أهداف DO-278A (خاص بالمحطات الأرضية)
│   ├── MIL-STD-1472H-checklist.md   ← قائمة مطابقة معيار HMI
│   └── NIST-800-53-mapping.md  ← خريطة الضوابط الأمنية
└── operations/
    ├── PreLaunchChecklist.md   ← قائمة ما قبل الإطلاق
    └── PostFlightReport-template.md  ← قالب تقرير ما بعد الطيران
```

## المسؤوليات

| الدور | المسؤولية |
|---|---|
| System Engineer | يمتلك `requirements/` و `design/` |
| Safety Officer | يمتلك `safety/` |
| Quality Manager | يمتلك `plans/` و `compliance/` |
| Flight Director | يمتلك `operations/` |

## دورة التحديث

1. كل تغيير في الكود يجب أن يُحدّث مصفوفة التتبع (`traceability.csv`)
2. أي إضافة لمتطلب جديد تحتاج معرّفاً (`REQ-M130-GCS-XXX`) وتحديث `SRS-M130GCS.md`
3. أي خطر جديد يحتاج تحديث `HazardLog.md` + `FMEA.md`
4. تُغلَق مراجعة أي PR فقط بعد اجتياز أداة `tools/validate-traceability.py`

## معايير الاعتماد المستهدفة

- **ISO 9001:2015** — نظام إدارة الجودة (أساس)
- **AS9100D** — إضافات الفضاء والطيران
- **DO-178C / ED-12C** DAL-C — البرمجيات الطيرانية (كمرجع تصميمي — المحطة ليست airborne لكن تُعامَل كداعمة للطيران)
- **DO-278A / ED-109A** — البرمجيات الأرضية لـ CNS/ATM (المعيار الأنسب مباشرة)
- **MIL-STD-1472H** — متطلبات هندسة عوامل بشرية
- **MIL-STD-882E** — سلامة الأنظمة
- **ISO/IEC 27001:2022** — إدارة أمن المعلومات
- **NIST SP 800-53 Rev. 5** — ضوابط أمنية فدرالية
- **ISO/IEC 25010:2011** — نموذج جودة البرمجيات
- **ISO/IEC/IEEE 29148:2018** — هندسة المتطلبات
- **ISO/IEC/IEEE 12207:2017** — دورة حياة البرمجيات
- **ECSS-Q-ST-80C Rev. 1** — ضمان جودة برمجيات الفضاء
- **ARINC 661-6** — واجهات عرض قمرة القيادة (مرجع للتنبيهات)

# DO-278A Objectives — Ground CNS/ATM

- **Document ID**: COMP-DO278A-001
- **Revision**: A

DO-278A / ED-109A هو معيار **البرمجيات الأرضية** لـ CNS/ATM، ومناسب مباشرة لـ M130 GCS. يختار Assurance Level (AL) بديلاً لـ DAL الطيراني:

| AL | Equivalent DAL | Description |
|---|---|---|
| AL1 | DAL-A | Catastrophic consequence |
| AL2 | DAL-B | Hazardous |
| AL3 | DAL-C | Major |
| AL4 | DAL-D | Minor |
| AL5 | DAL-E | No effect |
| AL6 | — | Assurance via usage |

**Target for M130 GCS**: **AL3** (نظير DAL-C).

## Objective Mapping
الأهداف مطابقة لـ DO-178C مع تعديلات:
- **COTS** (commercial off-the-shelf): Qt, QGroundControl. تُعالج عبر:
  - Assurance through usage (AL6 للأجزاء غير الحرجة)
  - Design assurance لمحيط القلب (Safety Kernel) — AL3
- **Adaptation data**: ملفات envelope/config موقَّعة تُعامَل كبيانات تكيف
- **Databases**: لا ينطبق (لا توجد قاعدة بيانات طيرانية)

## Service History Argument
QGroundControl لديه service history موثق — يُقبَل جزئياً لأجزاء غير-critical. الأجزاء الحرجة (Safety Kernel) **تُطوَّر تحت AL3 كاملاً** وبلا اعتماد على service history.

## خريطة الأهداف (مختصر)

| Table | Objectives | Status |
|---|---|---|
| A-1 Planning | 8 plans | ✓ Foundation |
| A-2 Development | 7 dev objectives | Partial (will complete per pillar) |
| A-3 Verification of reqs | 7 | Pending |
| A-4 Verification of design | 13 | Pending |
| A-5 Verification of coding/integration | 9 | Pending |
| A-6 Testing of integrated output | 5 | Pending |
| A-7 Verification of test results | 9 | Partial |
| A-8 CM | 6 | ✓ |
| A-9 QA | 5 | ✓ |
| A-10 Certification Liaison | 3 | Partial |
| A-11 Adaptation Data | 4 | Planned for envelope config |
| A-12 Software Life Cycle Data | — | Docs structure ✓ |

## Next Steps
- إضافة `docs/design/LLR/` للمتطلبات منخفضة المستوى لكل module في Pillar PRs
- إضافة `docs/verification/` لنتائج التحقق
- إنشاء SAS (Software Accomplishment Summary) لكل release

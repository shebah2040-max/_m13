# M130 GCS — Improvement Plan & Progress Tracker

> **Document ID**: PLN-M130-GCS-IMPROVEMENT-001
> **Owner**: Engineering (مع مراجعة Devin AI)
> **Purpose**: خطة تحسين/إصلاح/إكمال مبنية على التحليل العميق لـ `qgc/custom`
> (تقرير `qgc_custom_deep_analysis.md` المنشور في 2026‑04‑23).
>
> **هدف الوثيقة**: تتبع التقدم عبر الجلسات المتعاقبة بحيث لا يحتاج Devin إلى
> إعادة التحليل أو إعادة التفهيم في كل جلسة جديدة. كل جلسة تفتح هذه الوثيقة
> وتعرف فوراً ما أُنجز وما التالي ورقم الـPR لكل بند.

---

## كيف تستخدم هذه الوثيقة

1. في بداية كل جلسة: **اقرأ الجدول الرئيسي** ثم اختر البند التالي غير المنجز
   (حالة ⏳).
2. عند بدء العمل على بند: حدّث حالته إلى 🚧 وأضف رقم الـPR.
3. عند اندماج الـPR: حدّث الحالة إلى ✅ وأضف commit hash/PR number.
4. إذا وُجد بند جديد أثناء العمل: أضفه لنفس القسم مع رقم جديد بدلاً من إعادة ترقيم.

**رموز الحالة**:

- ⏳ معلَّق — لم يبدأ بعد
- 🚧 قيد التنفيذ — PR مفتوح أو عمل محلي
- ✅ منجز ومُدمَج
- ❌ مرفوض أو مؤجل — مع سبب

---

## الجدول الرئيسي — خريطة الإصلاحات

> **الترتيب حسب الأولوية** (🔴 حرج ← 🟠 مهم ← 🟡 جودة)
> **مرجع كل بند** يشير إلى فقرة `qgc_custom_deep_analysis.md` الموازية.

| # | الأولوية | البند | الحالة | PR | مرجع التحليل |
|---|----------|-------|--------|-----|---------------|
| **R1 — جولة الحرج** |
| R1.1 | 🔴 | إصلاح format-string UB في `tests/core/test_support.h` | ✅ | #TBD (PR #1) | §3.6 |
| R1.2 | 🔴 | إضافة `ImprovementPlan.md` + knowledge note | ✅ | #TBD (PR #1) | (جديد) |
| R1.3 | 🔴 | ربط `ConsoleSwitcher.qml` كواجهة أساسية (بعد LoginScreen) | ⏳ | — | §3.1 |
| R1.4 | 🔴 | إصدار Alert.Emergency عند `VersionCompat::MajorMismatch` | ⏳ | — | §3.7 |
| R1.5 | 🔴 | تفعيل `-Werror` في CI (بدون تأثير local) | 🚧 | #TBD (PR #2) | §3.16 |
| **R2 — إكمال الـ"delivered"** |
| R2.1 | 🟠 | حذف `RocketTelemetryFactGroup` + تهجير QML → `m130Gnc` | ⏳ | — | §3.4 |
| R2.2 | 🟠 | إضافة velocities إلى `M130GncState` (bump dialect) لتصحيح IIP | ⏳ | — | §3.8 |
| R2.3 | 🟠 | إغلاق/تحديث 12 مطلباً `Pending` في traceability | ⏳ | — | §3.9 |
| R2.4 | 🟠 | اختبارات QtQuickTest smoke لكل console في `tests/qml/` | ⏳ | — | §3.10 |
| R2.5 | 🟠 | قياس coverage فعلي بـlcov في CI | ⏳ | — | §3.11 |
| R2.6 | 🟠 | تشغيل `lupdate` واستكمال `m130_{ar,en}.ts` | ⏳ | — | §3.12 |
| R2.7 | 🟠 | إزالة 66 لوناً hardcoded + ثيم مركزي `QGCPaletteAerospace` | ⏳ | — | §3.2 |
| **R3 — أمان وتوافق** |
| R3.1 | 🟠 | تفعيل MAVLink v2 signing (REQ‑PROT‑001) + مفاتيح من SessionManager | ⏳ | — | §3.3 |
| R3.2 | 🟠 | إغلاق REQ‑SEC‑007 / 008 / 009 | ⏳ | — | §3.9 |
| R3.3 | 🟠 | اختبارات pytest لـ `tools/*.py` (لا سيما `generate-dialect.py`) | ⏳ | — | §3.18 |
| **R4 — جودة وأسلوب** |
| R4.1 | 🟡 | توحيد لغة التعليقات (إنكليزية في الكود، عربية في `docs/` فقط) | ⏳ | — | §3.13 |
| R4.2 | 🟡 | تصحيح نوع `CustomOptions::_plugin` إلى `CustomPlugin*` | ⏳ | — | §3.14 |
| R4.3 | 🟡 | استبدال `font.family:"monospace"` بـ `ScreenTools.monoFontFamily` | ⏳ | — | §3.15 |
| R4.4 | 🟡 | استبدال static singleton بـ Meyer's singleton في FirmwarePluginFactory | ⏳ | — | §3.20 |
| R4.5 | 🟡 | جعل `res/icons/rocket_icon.svg` symlink أو حذف التكرار | ⏳ | — | §3.5 |
| R4.6 | 🟡 | تبسيط `tests/core/CMakeLists.txt` بدالة helper | ⏳ | — | §3.19 |

---

## سجل الـPRs المُدمجة

| PR | العنوان | البنود | تاريخ الدمج | CI |
|----|---------|--------|-------------|-----|
| #1 | `[M130-PR1] Add Improvement Plan + fix test_support.h format UB` | R1.1, R1.2 | 2026-04-23 | 5 passed |
| #2 | `[M130-PR2] Enable -Werror -Wall -Wextra in CI configure step` | R1.5 | — | — |

*(يُحدَّث يدوياً بعد دمج كل PR. استخدم `git log --oneline -- qgc/custom/docs/plans/ImprovementPlan.md` لرؤية تاريخ تحديثات الوثيقة نفسها.)*

---

## نصائح للـDevin session التالية

1. **لا تُعيد التحليل** — الـtree صحيح ومتين. راجع فقط ما الذي تغيَّر منذ آخر PR.
2. **اتبع ترتيب الأولوية R1→R2→R3→R4** ما لم يطلب المستخدم غير ذلك.
3. **كل PR يجب**:
   - يلمس ≤ 3 بنود من هذه الخطة (حفاظاً على الحجم).
   - يُحدِّث حالة الـR-id المقابلة في الجدول أعلاه.
   - يمرر `tests/core` (الـ51 اختباراً) + `validate-traceability` + `validate-dialect` + `generate-dialect --check`.
   - ينطلق من فرع `devin/<timestamp>-<slug>` جديد.
4. **للألوان (R2.7)**: ابدأ بتعريف `m130Palette` context property في `CustomPlugin.cc`، ثم هجّر QML ملفاً ملفاً.
5. **للـConsoleSwitcher (R1.3)**: الحل الأبسط = تعديل `FlyViewCustomLayer.qml` ليصبح Loader على `ConsoleSwitcher.qml` عندما `m130Access.loggedIn === true`، وإلا `LoginScreen`.
6. **للـIIP velocities (R2.2)**: bumping `dialect` major = breaking change → نسّق مع مستهلكي m130.xml الآخرين قبل التنفيذ.
7. **ملاحظة مهمة حول الـdialect**: `m130.xml` و `src/protocol/generated/*` مربوطة بـCI check `generate-dialect.py --check`. أي تعديل يدوي على الـgenerated headers سيفشل الـCI.

## المقاييس الأولية (baseline) — 2026‑04‑23

```text
ملفات qgc/custom الكلية: 281
أسطر C++/headers:       8,682
ملفات QML:              20
اختبارات core:           51 / 51 تمرّ
مطالب Pending:           12 / 69
ألوان hardcoded في QML:  66
نصوص مترجَمة عربي / إنكليزي: 10 / 8
```

الهدف بعد اكتمال الـplan:

```text
مطالب Pending:           0
ألوان hardcoded:          0
اختبارات QML:            ≥ 7 (واحد لكل console)
coverage فعلي مُقاس:      ≥ 80% statement
-Werror:                  مفعَّل
```

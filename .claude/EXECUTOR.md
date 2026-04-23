---
noteId: "d5cde1703e8811f19f7a5d62fc7c3ce2"
tags: []

---

# EXECUTOR.md — نظام تعليمات التنفيذ
> هذا الملف يُقرأ من قبل **المنفذ** (Executor Agent) ويُكتب من قبل **المحلل** (Architect Agent)
> آخر تحديث: 2026-04-22 | الدورة: #3

---

## 0. من أنا وما دوري

أنا **المنفذ** (Executor). أنا الوكيل المسؤول عن:
- **تنفيذ التغييرات** في الكود بناءً على التعليمات المكتوبة أدناه **بدقة حرفية**
- **الإبلاغ** عن أي شيء غير متوقع أجده أثناء التنفيذ
- **عدم الاجتهاد** — أنفذ فقط ما هو مكتوب، لا أضيف ولا أحذف ولا أعدّل شيئاً غير مطلوب

**رموز الحالة**: ⬜ لم يُنفَّذ | 🔄 قيد التنفيذ | ✅ نُفِّذ وتُحقق منه | ❌ نُفِّذ لكن فيه خطأ

---

## 1. القواعد الإلزامية

### 1.1 قاعدة التنفيذ الحرفي
- **نفّذ فقط ما هو مكتوب في قسم "المهام الحالية"** أدناه
- لا تجتهد، لا تحسّن، لا تضف كوداً "أفضل" من المطلوب
- إذا وجدت شيئاً تعتقد أنه خطأ في التعليمات — **أبلغ ولا تصحح من عندك**

### 1.2 قاعدة Python المرجعية
- **لا تعدّل أي ملف Python** إلا إذا طُلب ذلك صراحةً في مهمة محددة
- Python (`m130_mpc_autopilot.py`) هي مصدر الحقيقة — C++ يجب أن يطابقها

### 1.3 قاعدة التحقق الذاتي
بعد كل تعديل، تحقق من:
1. **الأقواس**: كل `{` لها `}` مقابلة
2. **الفواصل المنقوطة**: كل سطر C++ ينتهي بـ `;`
3. **الأنواع**: `float` لها `f` suffix (مثل `0.8f` وليس `0.8`)

---

## 2. السياق (2026-04-22)

**الوضع الحالي**:
- الهيكل القديم `rocket_gnc_mpc/` أُعيد هيكلته إلى `rocket_mpc/` معيارياً
- جميع إصلاحات الدورة #1 مطبّقة في الكود الجديد
- Python↔C++ تطابق ~99%
- SITL يعمل بدقة 0.58% (فرق 15م فقط)
- **HITL لم يُشغَّل بعد الإصلاحات** — يحتاج بناء APK جديد

**المسارات الرئيسية** (نسبية من `m13/`):
```
PYTHON_MPC    = 6DOF_v4_pure/mpc/m130_mpc_autopilot.py
CPP_MODULE    = AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_mpc/RocketMPC.cpp
CPP_MPC_CTRL  = AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_mpc/mpc_controller.cpp
CPP_LOS       = AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_mpc/los_guidance.cpp
CPP_PARAMS    = AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_mpc/rocket_mpc_params.c
```

---

## 3. المهام الحالية — الدورة #3

> **لا توجد مهام برمجية معلقة حالياً.**

الخطوة التالية هي **تشغيل HITL** (مسؤولية المستخدم):
```bash
# 1. بناء APK (clean إلزامي)
cd $PROJECT_ROOT/AndroidApp && ./gradlew clean assembleDebug

# 2. تثبيت
adb install -r app/build/outputs/apk/debug/app-debug.apk

# 3. تشغيل HITL
adb shell am force-stop com.ardophone.px4v17
python3 sitl_hil_bridge_v3.py --hitl
adb shell am start-foreground-service -n com.ardophone.px4v17/.service.FlightService \
  -a com.ardophone.px4v17.START_FLIGHT --ez hitl true
```

**بعد التشغيل**: أبلغ المحلل بنتيجة المدى (range) للمقارنة مع 3258-3460م القديمة.

---

## 4. المهام المكتملة ✅

### [الدورة #1 — 2026-04-20 → مُطبَّقة ضمنياً في إعادة الهيكلة]

| # | الإصلاح | المكان في الكود الجديد | الحالة |
|---|---------|----------------------|--------|
| 1 | s_burn denominator 0.5→0.8 | `los_guidance.cpp:85` | ✅ مُطبَّق |
| 2 | tailoff Hermite fade (s_peak=0.67) | `mpc_controller.cpp:123-154` | ✅ مُطبَّق |
| 3 | thrust_plateau = I/(bt-0.75*tt) | `mpc_controller.cpp:70-94` | ✅ مُطبَّق |
| 4 | tf_use min = 2.0s | `mpc_controller.cpp:398` | ✅ مُطبَّق |
| 5 | لا r_w boost (Python لا يملكه) | N/A | ✅ صحيح |

---

## 5. تحذيرات

1. **لا تعدّل** ملفات Python أبداً
2. **لا تعدّل** `c_generated_code/` — هذه مولّدة بواسطة Acados
3. **لا تعدّل** ملفات `acados_generated/` داخل `rocket_mpc/` — مولّدة
4. عند بناء APK: استخدم `clean assembleDebug` دائماً — ليس `assembleDebug` وحدها
5. تحقق من timestamp APK بعد البناء (انظر CLAUDE.md الدرس 1)

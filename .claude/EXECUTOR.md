# EXECUTOR.md — نظام تعليمات التنفيذ
> هذا الملف يُقرأ من قبل **المنفذ** (Executor Agent) ويُكتب من قبل **المحلل** (Architect Agent)
> آخر تحديث: 2026-04-20 | الدورة: #1

---

## 0. من أنا وما دوري

أنا **المنفذ** (Executor). أنا الوكيل المسؤول عن:
- **تنفيذ التغييرات** في الكود بناءً على التعليمات المكتوبة أدناه **بدقة حرفية**
- **الإبلاغ** عن أي شيء غير متوقع أجده أثناء التنفيذ
- **عدم الاجتهاد** — أنفذ فقط ما هو مكتوب، لا أضيف ولا أحذف ولا أعدّل شيئاً غير مطلوب

**أنا لا أحلل ولا أقرر** — التحليل والقرارات مسؤولية المحلل (ARCHITECT.md).

### علاقتي بالملفات الأخرى

| الملف | علاقتي به |
|-------|-----------|
| `CLAUDE.md` | أقرأه لفهم المشروع — **لا أعدّله أبداً** |
| `ARCHITECT.md` | أقرأه لفهم السياق — **لا أعدّله أبداً** |
| `EXECUTOR.md` (هذا الملف) | أقرأ المهام وأنفذها — **لا أعدّله** (المحلل هو من يحدّثه) |
| ملفات المشروع | أعدّلها **فقط** حسب التعليمات أدناه |

### عند بدء جلسة جديدة
1. اقرأ `CLAUDE.md` — افهم المشروع والمسارات
2. اقرأ `ARCHITECT.md` — افهم أين وصل التحليل
3. اقرأ هذا الملف — نفّذ المهام ذات الحالة ⬜ فقط **بالترتيب**
4. بعد إتمام جميع المهام: اتبع قسم "بعد إتمام جميع المهام" (قسم 7)

**رموز الحالة**: ⬜ لم يُنفَّذ | 🔄 قيد التنفيذ | ✅ نُفِّذ وتُحقق منه | ❌ نُفِّذ لكن فيه خطأ

---

## 1. القواعد الإلزامية (يجب قراءتها قبل أي عمل)

### 1.1 قاعدة التنفيذ الحرفي
- **نفّذ فقط ما هو مكتوب في قسم "المهام الحالية"** أدناه
- لا تجتهد، لا تحسّن، لا تضف كوداً "أفضل" من المطلوب
- إذا وجدت شيئاً تعتقد أنه خطأ في التعليمات — **أبلغ ولا تصحح من عندك**
- نفّذ المهام **بالترتيب المذكور** ما لم يُذكر خلاف ذلك

### 1.2 قاعدة Python المرجعية
- **لا تعدّل أي ملف Python** إلا إذا طُلب ذلك صراحةً في مهمة محددة
- Python (`m130_mpc_autopilot.py`) هي مصدر الحقيقة — C++ يجب أن يطابقها
- إذا وجدت تعارضاً بين التعليمات هنا و Python — أبلغ المحلل

### 1.3 قاعدة عدم التلوث
- **لا تضف تعليقات** غير موجودة في التعليمات
- **لا تضف docstrings أو أنواع بيانات** غير مطلوبة
- **لا تعيد تنسيق** كود لم تُطلب منك تعديله
- **لا تنظّف** imports أو متغيرات غير مستخدمة
- **لا تحذف** كوداً غير مطلوب حذفه

### 1.4 قاعدة التحقق الذاتي
بعد كل تعديل، تحقق من:
1. **الأقواس**: كل `{` لها `}` مقابلة
2. **الفواصل المنقوطة**: كل سطر C++ ينتهي بـ `;` (إلا الشروط والحلقات)
3. **الأنواع**: `float` لها `f` suffix (مثل `0.8f` وليس `0.8`)
4. **عدم وجود** كود مكرر أو أسطر مفقودة

### 1.5 قاعدة الإبلاغ
أبلغ المحلل (في ردك) إذا:
- وجدت أن الكود القديم المذكور في المهمة **لا يطابق** ما في الملف فعلياً
- وجدت أن السطر المذكور **ليس** في المكان المحدد
- وجدت خطأ syntax ناتج عن التعديل
- وجدت أي شيء آخر غير متوقع

---

## 2. السياق (اقرأ قبل التنفيذ)

**المشروع**: نظام MPC لصاروخ M130. Python هي التنفيذ المرجعي، C++ هي إعادة تنفيذ يدوية يجب أن تطابقها.

**المشكلة**: تم اكتشاف 5 فروقات بين Python و C++ تسبب اختلافاً في سلوك المحاكاة:
- SITL pitch@5s = 7.5° بدلاً من 4.8° (Python)
- HITL range = 3400م بدلاً من 2600م (الهدف)

**الهدف**: جعل C++ مطابقاً لـ Python حتى تتطابق نتائج SITL مع Python-only.

### المسارات الكاملة للملفات المستهدفة

> **تنبيه**: المسارات أدناه نسبية من PROJECT_ROOT (انظر CLAUDE.md قسم 0.1). حدد PROJECT_ROOT أولاً.

| الاسم المختصر | المسار النسبي من PROJECT_ROOT |
|---------------|-------------------------------|
| **المرجع (Python)** | `6DOF_v4_pure/mpc/m130_mpc_autopilot.py` |
| **RocketGncMpc.cpp** | `AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_gnc_mpc/RocketGncMpc.cpp` |
| **RocketGncMpc.hpp** | `AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_gnc_mpc/RocketGncMpc.hpp` |
| **rocket_gnc_mpc_params.c** | `AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_gnc_mpc/rocket_gnc_mpc_params.c` |

---

## 3. المهام الحالية — الدورة #1

### مهمة 1: إصلاح s_burn transition speed [D-003]
**الحالة**: ⬜ لم يُنفَّذ

- **الملف**: `RocketGncMpc.cpp`
- **السطر**: ~743
- **الكود القديم** (ابحث عن هذا النص بالضبط):
```cpp
	_s_burn = 0.5f * (1.0f + tanhf(t_rel_burn / 0.5f));
```
- **الكود الجديد** (استبدل به):
```cpp
	_s_burn = 0.5f * (1.0f + tanhf(t_rel_burn / 0.8f));
```
- **السبب**: Python يستخدم مقام 0.8. C++ يستخدم 0.5 خطأً. هذا يغير توقيت الانتقال من boost إلى cruise.
- **التحقق بعد التعديل**: ابحث عن `tanhf(t_rel_burn` — يجب أن يكون `/0.8f)` وليس `/0.5f)`

---

### مهمة 2: إصلاح Tailoff Weight Scheduling [D-001]
**الحالة**: ⬜ لم يُنفَّذ

- **الملف**: `RocketGncMpc.cpp`
- **الدالة**: `_compute_weights()`
- **المكان**: بلوك `} else if (in_tailoff) {` (سطر ~1390)
- **الكود القديم** (استبدل كامل البلوك من `} else if (in_tailoff) {` حتى القوس `}` الذي قبل `} else {`):
```cpp
	} else if (in_tailoff) {
		// Tail-off transition (match Python: quadratic fade + heavy damping)
		const float s_to = math::constrain((t - t_tailoff_start) / (t_tailoff_end - t_tailoff_start), 0.0f, 1.0f);
		gamma_base = 250.0f * (1.0f - s_to) * (1.0f - s_to) + 2.0f;
		q_w = 200.0f + 800.0f * s_to;
		r_w = 80.0f + 200.0f * s_to;
		de_rate_w = 120.0f * (1.0f - s_to) + 200.0f * s_to;
		dr_rate_w = 60.0f + 100.0f * s_to;
		alpha_w = 120.0f + 800.0f * s_to;
```
- **الكود الجديد** (استبدل به):
```cpp
	} else if (in_tailoff) {
		// Tail-off transition (match Python exactly: 2-phase with Hermite fade)
		const float s_to = math::constrain((t - t_tailoff_start) / (t_tailoff_end - t_tailoff_start), 0.0f, 1.0f);
		const float s_peak = 0.67f;
		const float s_up = fminf(1.0f, s_to / s_peak);

		// Phase 1: ramp to peak damping (0 -> s_peak)
		const float gamma_act = 250.0f * (1.0f - s_up) * (1.0f - s_up) + 2.0f;
		const float q_act     = 200.0f + 800.0f * s_up;
		const float r_act     = 80.0f + 200.0f * s_up;
		const float de_act    = 120.0f * (1.0f - s_up) + 200.0f * s_up;
		const float dr_act    = 60.0f + 100.0f * s_up;
		const float alpha_act = 120.0f + 800.0f * s_up;

		if (s_to > s_peak) {
			// Phase 2: Hermite fade from peak -> coast values
			const float s_f = (s_to - s_peak) / (1.0f - s_peak);
			const float h = 3.0f * s_f * s_f - 2.0f * s_f * s_f * s_f;
			gamma_base = gamma_act + (100.0f - gamma_act) * h;
			q_w        = q_act    + (60.0f  - q_act)     * h;
			r_w        = r_act    + (40.0f  - r_act)     * h;
			de_rate_w  = de_act   + (20.0f  - de_act)    * h;
			dr_rate_w  = dr_act   + (20.0f  - dr_act)    * h;
			alpha_w    = alpha_act + (40.0f  - alpha_act) * h;
		} else {
			gamma_base = gamma_act;
			q_w        = q_act;
			r_w        = r_act;
			de_rate_w  = de_act;
			dr_rate_w  = dr_act;
			alpha_w    = alpha_act;
		}
```
- **السبب**: Python يستخدم مرحلتين — الأولى ترفع damping إلى الذروة (s_peak=0.67)، الثانية تُنزله بسلاسة Hermite إلى قيم coast. C++ القديم كان يسبب gamma_base=2 و q_w=1000 عند نهاية tailoff بدلاً من 100 و 60.
- **التحقق بعد التعديل**: حساب ذهني عند s_to=1.0 → gamma_base≈100, q_w≈60, r_w≈40 (قيم coast)

---

### مهمة 3: إضافة thrust_plateau [D-002]
**الحالة**: ⬜ لم يُنفَّذ

هذه مهمة من **3 أجزاء** — نفذها بالترتيب:

#### الجزء أ: إضافة المعامل في params.c
- **الملف**: `rocket_gnc_mpc_params.c`
- **المكان**: بعد تعريف `RMPC_THRUST` مباشرةً (بعد سطر `PARAM_DEFINE_FLOAT(RMPC_THRUST, 753.0f);`)
- **أضف هذا النص بعده** (سطر فارغ ثم البلوك):
```c

/**
 * Plateau thrust during non-tailoff boost (N)
 *
 * Computed as total_impulse / (burn_time - 0.75 * t_tail).
 * Higher than average thrust (~18.6%) — matches mid-burn reality.
 * Used by MPC solver as thrust parameter during non-tailoff boost.
 *
 * @group Rocket MPC
 * @unit N
 * @min 0
 * @max 2000
 * @decimal 1
 */
PARAM_DEFINE_FLOAT(RMPC_THRUST_P, 893.5f);
```
- **⚠️ لا تحذف RMPC_THRUST الأصلي** — هو يُستخدم في أماكن أخرى

#### الجزء ب: إضافة المعامل في الهيدر
- **الملف**: `RocketGncMpc.hpp`
- **المكان**: داخل `DEFINE_PARAMETERS(...)` — بعد سطر `_param_thrust` مباشرةً
- **ابحث عن**:
```cpp
		(ParamFloat<px4::params::RMPC_THRUST>)     _param_thrust,
```
- **أضف سطراً جديداً بعده مباشرة**:
```cpp
		(ParamFloat<px4::params::RMPC_THRUST_P>)   _param_thrust_plateau,
```

#### الجزء ج: استخدام thrust_plateau في _get_params
- **الملف**: `RocketGncMpc.cpp`
- **الدالة**: `_get_params()`
- **ابحث عن**:
```cpp
	const float thrust_n  = _param_thrust.get();
```
- **استبدل بـ**:
```cpp
	const float thrust_n  = _param_thrust_plateau.get();
```
- **السبب**: Python يمرر 893.5N إلى MPC solver. C++ كان يمرر 753N. فرق 140N (18.6%) يؤثر مباشرة على نموذج الديناميكا.
- **التحقق بعد التعديل**: ابحث عن `_param_thrust_plateau.get()` في _get_params — يجب أن يوجد

---

### مهمة 4: إصلاح tf_use minimum [D-004]
**الحالة**: ⬜ لم يُنفَّذ

- **الملف**: `RocketGncMpc.cpp`
- **السطر**: ~845
- **ابحث عن**:
```cpp
		tf_use = fmaxf(1.0f, fminf(4.0f, t_to_target * 0.8f));
```
- **استبدل بـ**:
```cpp
		tf_use = fmaxf(2.0f, fminf(4.0f, t_to_target * 0.8f));
```
- **السبب**: Python يستخدم min=2.0s. أفق أقصر من 2 ثانية يسبب عدم استقرار قرب الهدف.
- **التحقق بعد التعديل**: ابحث عن `fmaxf(2.0f, fminf(4.0f` — يجب أن يوجد

---

### مهمة 5: إزالة roll rate r_w boost [D-005]
**الحالة**: ⬜ لم يُنفَّذ

- **الملف**: `RocketGncMpc.cpp`
- **الدالة**: `_compute_weights()`
- **المكان**: بعد بلوك `alpha_w += ...` (سطر ~1426)
- **ابحث عن هذا البلوك** (6 أسطر) **واحذفه بالكامل**:
```cpp
		// ===== Adaptive roll rate damping (SITL robustness) =====
		// When roll rate grows, boost r_w too (yaw-roll coupling)
		const float p_rate_abs = fabsf((float)x0[3]) * 57.2958f;
		if (p_rate_abs > 5.0f) {
			r_w += 100.0f * fminf(p_rate_abs / 20.0f, 3.0f);
		}
```
- **السبب**: هذا كود C++ فقط — غير موجود في Python المرجعي. يغير سلوك MPC بشكل غير متوقع.
- **التحقق بعد التعديل**: ابحث عن `p_rate_abs` في `_compute_weights()` — يجب ألا يوجد

---

## 4. المهام المكتملة والمتحقق منها ✅

> تُنقل هنا المهام بعد أن يتحقق **المحلل** من صحة التنفيذ.
> كل مهمة تُنقل مع تاريخ الإتمام ونتيجة التحقق.

(فارغ — لم يُنفَّذ شيء بعد)

---

## 5. تحذيرات عامة

1. **لا تعدّل** أي ملف Python — أبداً
2. **لا تعدّل** `_forward_guess()` — المنطق متطابق بين Python و C++
3. **لا تعدّل** phi blending في warm start (سطر ~896-909) — مؤجل عمداً (Q-003)
4. **لا تعدّل** أي HITL-specific code (كل ما يتعلق بـ `_sys_hitl`) — سيُراجع لاحقاً
5. **لا تحذف** RMPC_THRUST الأصلي (753) — أضف RMPC_THRUST_P بجانبه
6. **لا تعدّل** أي ملف خارج المجلد `rocket_gnc_mpc/`
7. بعد كل تعديل: **تأكد** من عدم وجود أقواس مفقودة أو كود مكرر

---

## 6. ملخص التعديلات للتحقق السريع

> هذا الجدول يُستخدم من المحلل للتحقق بسرعة بعد التنفيذ

| # | الملف | نوع التعديل | ماذا تبحث عنه بعد التنفيذ | الحالة |
|---|-------|-------------|---------------------------|--------|
| 1 | RocketGncMpc.cpp | استبدال | `tanhf(t_rel_burn / 0.8f)` | ⬜ |
| 2 | RocketGncMpc.cpp | استبدال بلوك | `s_peak = 0.67f` + `Hermite fade` | ⬜ |
| 3a | rocket_gnc_mpc_params.c | إضافة | `RMPC_THRUST_P, 893.5f` | ⬜ |
| 3b | RocketGncMpc.hpp | إضافة سطر | `_param_thrust_plateau` | ⬜ |
| 3c | RocketGncMpc.cpp | استبدال | `_param_thrust_plateau.get()` | ⬜ |
| 4 | RocketGncMpc.cpp | استبدال | `fmaxf(2.0f, fminf(4.0f` | ⬜ |
| 5 | RocketGncMpc.cpp | حذف | عدم وجود `p_rate_abs` في _compute_weights | ⬜ |

---

## 7. بعد إتمام جميع المهام

عند الانتهاء من تنفيذ جميع المهام الخمس:

1. **أبلغ المحلل** بقائمة ما تم تنفيذه وأي ملاحظات أو مشاكل واجهتها
2. **لا تبنِ APK** — البناء مسؤولية المستخدم أو المحلل
3. **لا تشغّل محاكاة** — التشغيل والتحقق مسؤولية المحلل
4. **لا تعدّل ARCHITECT.md أو EXECUTOR.md** — المحلل هو من يحدّثهما بعد التحقق

**ما يفعله المحلل بعد ذلك** (ليس مسؤوليتك):
- يقرأ الملفات المعدّلة فعلياً ويقارنها بالتعليمات
- يسجّل نتيجة التحقق في ARCHITECT.md
- يُحدّث حالة المهام هنا (⬜ → ✅ أو ❌)
- إذا نجح: ينتقل للبناء والمحاكاة
- إذا فشل: يكتب مهمة تصحيحية جديدة

# CLAUDE.md — M130 GNC MPC: المرجع الثابت
> آخر تحديث: 2026-04-20
> هذا الملف يحتوي **فقط** على المعلومات الثابتة للمشروع (فيزياء، رياضيات، بنية، أوامر)
> كل المعلومات المتغيرة (حالة، اكتشافات، نتائج، خطط) موجودة في `.claude/ARCHITECT.md`

---

## 0. نظام العمل

### 0.1 المسار المرجعي
```
PROJECT_ROOT = مسار مجلد m13 على الجهاز الحالي
```
**ملاحظة**: المسار يختلف حسب الجهاز. عند بدء جلسة جديدة، حدد PROJECT_ROOT أولاً بالبحث عن مجلد يحتوي `6DOF_v4_pure/` و `AndroidApp/` و `sitl_hil_bridge_v3.py`.

**المسارات المتكررة** (نسبية من PROJECT_ROOT):
```
PYTHON_MPC  = 6DOF_v4_pure/mpc/m130_mpc_autopilot.py
CPP_MPC     = AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_gnc_mpc/RocketGncMpc.cpp
CPP_HPP     = AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_gnc_mpc/RocketGncMpc.hpp
CPP_PARAMS  = AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_gnc_mpc/rocket_gnc_mpc_params.c
CPP_MHE     = AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_mhe_nav/RocketMheNav.cpp
BRIDGE      = sitl_hil_bridge_v3.py
```

### 0.2 نظام ملفات العمل (.claude/)

| الملف | المحتوى | من يكتبه | من يقرأه |
|-------|---------|----------|----------|
| `CLAUDE.md` (هذا الملف) | معلومات ثابتة: فيزياء، رياضيات، بنية، أوامر | المحلل (نادراً) | الجميع |
| `.claude/ARCHITECT.md` | معلومات متغيرة: حالة، اكتشافات، نتائج، خطط، قرارات | المحلل | المحلل + المستخدم |
| `.claude/EXECUTOR.md` | تعليمات تنفيذ دقيقة للمهام الحالية فقط | المحلل | المنفذ + المحلل |

**دورة العمل**:
```
المحلل يحلل ويكتشف → يوثّق في ARCHITECT.md → يكتب مهام في EXECUTOR.md
  → المنفذ ينفذ → المحلل يتحقق ويوثّق النتيجة → يكرر أو ينتقل لدورة جديدة
```

**القاعدة الذهبية**: Python (`m130_mpc_autopilot.py`) هي المرجع دائماً. C++ يجب أن يطابقها. لا يُعدَّل Python ليطابق C++ — أبداً.

### 0.3 متى يُحدَّث هذا الملف (CLAUDE.md)

يُحدَّث **فقط** عند:
1. تغيّر في **الفيزياء** (صاروخ جديد، محرك مختلف، تكوين زعانف مختلف)
2. تغيّر في **النموذج الرياضي** (أبعاد جديدة، نوع solver مختلف)
3. تغيّر في **بنية المجلدات** (ملف جديد مهم، إعادة هيكلة)
4. تغيّر في **أوامر البناء** (أداة جديدة، مسار مختلف)
5. اكتشاف **درس مستفاد** منهجي جديد (خطأ تكرر وله قاعدة عامة)

**لا يُحدَّث** عند: تغيّر حالة مرحلة، اكتشاف فرق بين Python و C++، نتيجة محاكاة جديدة — هذه تذهب لـ ARCHITECT.md.

---

## 1. القواعد الإلزامية العامة

### 1.1 اللغة
- **دائماً أجب بالعربية**

### 1.2 لا تعديل بدون خطة
- **يجب التخطيط أولاً** وعرض الخطة على المستخدم قبل أي تعديل
- **اختبر بالترتيب**: Python-only → SITL → HITL
- **لا تقل "تم"** حتى تتأكد بالأرقام من نتائج المحاكاة الفعلية

### 1.3 قبل أي تعديل على ملف
1. اقرأ الملف المستهدف بالكامل
2. اقرأ الملف المقابل في التنفيذ الآخر (Python ↔ C++)
3. تحقق من تطابق الأبعاد (NX, NU, NP, NY, NYN)
4. تحقق من تطابق الثوابت العددية
5. اعرض الخطة أولاً

### 1.4 دروس مستفادة (أخطاء منهجية يجب عدم تكرارها)

> هذا القسم يُحدَّث فقط عند اكتشاف خطأ **منهجي** جديد — وليس أخطاء فردية

**الدرس 1: تحقق من timestamp الـ APK**
- APK بُني 11:43:53 → CPP عُدّل 11:43:56 → الإصلاح لم يدخل
- **القاعدة**: دائماً تحقق بـ `stat` أن APK أحدث من CPP، أو استخدم `clean build`

**الدرس 2: لا تقدم فرضية كحقيقة**
- الادعاء: TCP latency → MHE pitch lag → +400m
- الواقع: الخطأ كان +830m ومع الإصلاح لا يزال +800m
- **القاعدة**: ميّز دائماً بين "أعتقد" و "تأكدت بالأرقام". وثّق الفرضيات في ARCHITECT.md مع علامة [فرضية].

**الدرس 3: التنفيذ الحرفي يمنع الأخطاء**
- **القاعدة**: المنفذ لا يجتهد — ينفذ بالضبط ما في EXECUTOR.md. أي "تحسين" غير مطلوب هو مصدر خطأ محتمل.

---

## 2. هوية المشروع

**ما هو**: صاروخ موجّه (M130 / قبضة1) بنظام تحكم MPC + تقدير حالة MHE، كلاهما مولّد بـ **Acados** من نموذج Python واحد.

**الهدف**: إصابة هدف أرضي على بعد **2600 م** بخطأ **< 5 م** عبر جميع المراحل.

### المراحل الأربع

| المرحلة | الاسم | المتحكم | الفيزياء | الحساسات |
|---------|-------|---------|----------|----------|
| 1 | Python-only | Python MPC (acados wrapper) | Python 6DOF | مثالية (حالة المحاكاة مباشرة) |
| 2 | SITL | PX4 C++ module (acados C) | Python 6DOF عبر bridge | Sim → MAVLink HIL (localhost) |
| 3 | HITL | Android phone (acados C) | Python 6DOF عبر TCP bridge | Sim → MAVLink عبر TCP (USB Ethernet) |
| 4 | طيران حقيقي | Android phone (acados C) | فيزياء حقيقية | IMU/GPS حقيقي |

> **حالة** كل مرحلة (يعمل/معطّل/نتائج) موثّقة في `.claude/ARCHITECT.md` قسم "الحالة الحالية"

**ملاحظة حرجة**: المراحل 2-4 تستخدم **نفس كود Acados C المولّد** لكن **تنفيذ C++ مختلف** (reimplementation يدوي) لمنطق الأوزان والتوجيه والمعاملات.

---

## 3. النظام الفيزيائي

### 3.1 الصاروخ: قبضة1 (Qabthah1)
- الكتلة: 12.74 كجم (ممتلئ) → 11.11 كجم (جاف) — وقود 1.63 كجم
- القطر: 130 مم، الطول: 1.094 م
- تكوين الزعانف: **X-configuration**, زعانف خلفية (tail), 4 زعانف
- السيرفو: KST X20-7.4-M-830 brushless, tau=0.015s, حد ±20°, سرعة 270°/s
- الدفع: ~753 N متوسط, زمن احتراق ~4.77s, محرك صلب
- مركز الثقل: جاف [0.5684, 0, 0]م، وقود [0.5972, 0, 0]م
- الدفع من thrust_curve.csv: total_impulse=3593.24Ns, burn_time=4.7716s

### 3.2 القصور الذاتي

| المعامل | YAML (rocket_properties) | PX4 (params.c) | ملاحظة |
|---------|--------------------------|-----------------|--------|
| Ixx_wet | 0.0389 | 0.0353 | فرق -9.3% |
| Iyy_wet | 1.1651 | 1.0567 | فرق -9.3% |
| Izz_wet | 1.166 | (يستخدم Iyy) | — |
| Ixx_dry | 0.0356 | 0.0323 | فرق -9.3% |
| Iyy_dry | 1.0789 | 0.943 | فرق -12.6% |
| Izz_dry | 1.0779 | (يستخدم Iyy) | — |

> **تنبيه**: هذا التعارض موثّق كقرار معلق في ARCHITECT.md. قيم PX4 تُستخدم في MHE فقط. قيم YAML تُستخدم في Python 6DOF sim.

### 3.3 ملف الطيران
- الإطلاق: 15° ارتفاع، ارتفاع ~1200م فوق البحر
- الدفع (boost): 0-4.77s — انتقال s_burn بدالة tanh
- الانزلاق (coast/cruise): تثبيت ارتفاع عند القمة
- الغوص النهائي (terminal): يبدأ عند 65% من المدى (p0_dive)
- زاوية اصطدام مطلوبة: -30°
- الهدف: 2600م downrange, -20م ارتفاع, 0م crossrange

---

## 4. النموذج الرياضي (Acados)

### 4.1 MPC
- **الحالات (NX=18)**: V, gamma, chi, p, q, r, alpha, beta, phi, h/100, x/1000, y/1000, delta_e_cmd, delta_r_cmd, delta_a_cmd, delta_e_act, delta_r_act, delta_a_act
- **التحكم (NU=3)**: ddelta_e, ddelta_r, ddelta_a (معدلات تغيير الزعانف, rad/s)
- **المعاملات (NP=2)**: mass, thrust (متغيرة أثناء التشغيل)
- **الأفق**: N=80 عقدة, tf=4.0s (يتقلص ديناميكياً قرب الهدف)
- **المحلّل**: SQP_RTI, PARTIAL_CONDENSING_HPIPM, ERK integrator (4 مراحل, خطوتين)
- **التكلفة**: LINEAR_LS — 12 مخرج (stage: NY=12), 9 مخرجات (terminal: NYN=9)
- **القيود**: ناعمة على الحالات الزاوية (indices 0-6), صلبة على الزعانف (7-12), polytopic لحدود X-config

### 4.2 MHE
- **الحالات (NX=17)**: V, gamma, chi, p, q, r, alpha, beta, phi, h/100, x/1000, y/1000, bias_gx, bias_gy, bias_gz, wind_n, wind_e
- **المعاملات (NP=9)**: delta_e_act, delta_r_act, delta_a_act, mass, thrust, Ixx, Iyy, Izz, launch_alt
- **القياسات (NY=30)**: [y(13), noise_ref(17)] — y = [gyro(3), accel(3), baro(1), gps_pos(3), gps_vel(3)]
- **الأفق**: N=20 عقدة, 50 Hz
- **الغرض**: تقدير الرياح وانحراف الجيروسكوب

### 4.3 تطابق أبعاد Acados — مؤكد ✅

تم التحقق من 3 نسخ لكل header — **جميعها متطابقة بايت بايت** (تاريخ التحقق: 2026-04-20):

| المحلّل | NX | NU | NP | NY | NYN/NY0 | N |
|---------|----|----|----|----|---------|---|
| MPC | 18 | 3 | 2 | 12 | NYN=9 | 80 |
| MHE | 17 | 17 | 9 | 30 | NY0=47 | 20 |

### 4.4 خط أنابيب توليد الكود
```
Python model (m130_acados_model.py)
    → OCP setup (m130_ocp_setup.py)
        → AcadosOcpSolver → C code → c_generated_code/
            → نسخ يدوي إلى: rocket_gnc_mpc/acados_generated/
            → بناء APK (acados مترجم لـ ARM64)
```
**الثوابت الحرجة**: h مقسوم على 100, x مقسوم على 1000, y مقسوم على 1000 (normalization).
**ديناميكا السيرفو**: delta_dot_act = (delta_cmd - delta_act) / tau_servo
**القوى الهوائية**: تستخدم مواقع الزعانف **الفعلية** (وليس المأمورة) — MPC "يعرف" عن تأخر السيرفو.

---

## 5. البنية البرمجية

### 5.1 شجرة المجلدات (نسبية من PROJECT_ROOT)
```
├── 6DOF_v4_pure/                    # المرحلة 1: محاكاة Python
│   ├── rocket_6dof_sim.py           # محرك 6DOF الرئيسي
│   ├── dynamics/                     # وحدات الفيزياء
│   ├── mpc/                          # كود MPC/MHE Python
│   │   ├── m130_acados_model.py     # النموذج الرمزي CasADi
│   │   ├── m130_ocp_setup.py        # إعداد OCP
│   │   ├── m130_mpc_autopilot.py    # ★ التنفيذ المرجعي (MpcController)
│   │   ├── m130_mhe_model.py        # نموذج MHE
│   │   ├── m130_mhe_ocp_setup.py    # إعداد MHE OCP
│   │   └── m130_mhe_estimator.py    # مقدّر MHE
│   ├── config/6dof_config_advanced.yaml
│   ├── data/rocket_models/Qabthah1/ # بيانات الصاروخ (CSV + YAML)
│   ├── c_generated_code/            # كود Acados C المولّد
│   └── results/                     # نتائج المحاكاة
│
├── sitl_hil_bridge_v3.py            # الجسر بين المحاكاة و PX4
│
├── AndroidApp/                       # المرحلة 3: تطبيق Android
│   └── app/src/main/cpp/
│       ├── PX4-Autopilot/src/modules/
│       │   ├── rocket_gnc_mpc/       # ★ وحدة MPC C++ (يجب أن تطابق Python)
│       │   │   ├── RocketGncMpc.cpp
│       │   │   ├── RocketGncMpc.hpp
│       │   │   ├── rocket_gnc_mpc_params.c
│       │   │   └── acados_generated/
│       │   └── rocket_mhe_nav/       # وحدة MHE C++
│       │       ├── RocketMheNav.cpp
│       │       ├── RocketMheNav.hpp
│       │       ├── rocket_mhe_nav_params.c
│       │       └── acados_generated/
│       ├── px4_jni.cpp
│       └── acados_generated/         # نسخة ثالثة مشتركة
│
└── acados-main/                      # مكتبة Acados (Linux + ARM64)
```

### 5.2 كيف تعمل كل مرحلة

**المرحلة 1 (Python-only)**:
- `MpcController` في `m130_mpc_autopilot.py` — **التنفيذ المرجعي**
- يبني Acados solver, يحسب LOS guidance, 3 مراحل طيران (boost → cruise → terminal)
- **thrust_plateau**: محسوب = total_impulse / (burn_time - 0.75 * t_tail) ≈ **893.5N** — ليس 753 مباشرة
- تدفق: `control_function(state_dict, t) → fins[4]`

**المرحلة 2 (SITL)**:
- PX4 يعمل على Linux PC
- Bridge يرسل HIL_SENSOR + HIL_STATE_QUATERNION + HIL_GPS عبر TCP localhost
- يستقبل HIL_ACTUATOR_CONTROLS
- Warmup: 12s بيانات ثابتة, auto-arm, إطلاق عند t=3s

**المرحلة 3 (HITL)**:
- PX4 يعمل على **هاتف Samsung** كـ APK
- Bridge مع `--hitl` flag
- اتصال USB Ethernet: PC (10.42.0.1) ↔ Phone (10.42.0.174)
- `SYS_HITL=1` يفعّل تعديلات خاصة (gamma_ref bias, gamma_base, h_cruise_w)

---

## 6. الملفات الرئيسية

### أولوية 1 — اقرأ قبل أي تعديل

| الملف | المسار النسبي | الدور |
|-------|---------------|-------|
| النموذج الرياضي | `6DOF_v4_pure/mpc/m130_acados_model.py` | مصدر الحقيقة للفيزياء |
| إعداد OCP | `6DOF_v4_pure/mpc/m130_ocp_setup.py` | القيود والتكاليف |
| **MPC Python** | `6DOF_v4_pure/mpc/m130_mpc_autopilot.py` | **★ التنفيذ المرجعي** |
| **MPC C++** | `AndroidApp/.../rocket_gnc_mpc/RocketGncMpc.cpp` | **يجب أن يطابق Python** |
| MPC C++ header | `AndroidApp/.../rocket_gnc_mpc/RocketGncMpc.hpp` | أبعاد ومتغيرات |

### أولوية 2

| الملف | المسار النسبي | الدور |
|-------|---------------|-------|
| Bridge | `sitl_hil_bridge_v3.py` | الجسر بين المحاكاة و PX4 |
| PX4 params | `AndroidApp/.../rocket_gnc_mpc/rocket_gnc_mpc_params.c` | قيم المعاملات |
| MHE C++ | `AndroidApp/.../rocket_mhe_nav/RocketMheNav.cpp` | المقدّر |
| Acados MPC header | `6DOF_v4_pure/c_generated_code/acados_solver_m130_rocket.h` | أبعاد المحلّل |

### أولوية 3

| الملف | المسار النسبي | الدور |
|-------|---------------|-------|
| Config | `6DOF_v4_pure/config/6dof_config_advanced.yaml` | إعدادات المحاكاة |
| Rocket props | `6DOF_v4_pure/data/rocket_models/Qabthah1/rocket_properties.yaml` | خصائص فيزيائية |
| JNI | `AndroidApp/app/src/main/cpp/px4_jni.cpp` | معاملات Android |

---

## 7. أوامر البناء والتشغيل

> استبدل `$PROJECT_ROOT` بالمسار الفعلي على جهازك

```bash
# === بناء APK (clean — إلزامي!) ===
cd $PROJECT_ROOT/AndroidApp && ./gradlew clean assembleDebug

# === تثبيت APK ===
adb install -r app/build/outputs/apk/debug/app-debug.apk

# === تشغيل HITL ===
adb shell am force-stop com.ardophone.px4v17
cd $PROJECT_ROOT && python3 sitl_hil_bridge_v3.py --hitl
adb shell am start-foreground-service -n com.ardophone.px4v17/.service.FlightService \
  -a com.ardophone.px4v17.START_FLIGHT --ez hitl true

# === تشغيل SITL ===
cd PX4-Autopilot && rm -f build/px4_sitl_default/rootfs/parameters.bson
export LD_LIBRARY_PATH=$PROJECT_ROOT/acados-main/lib:$LD_LIBRARY_PATH
PX4_SYS_AUTOSTART=23000 ./build/px4_sitl_default/bin/px4
# (terminal آخر)
cd $PROJECT_ROOT && python3 sitl_hil_bridge_v3.py

# === تشغيل Python-only ===
cd $PROJECT_ROOT/6DOF_v4_pure && python3 rocket_6dof_sim.py

# === التحقق من أن APK أحدث من المصدر ===
stat -c '%Y %n' $PROJECT_ROOT/AndroidApp/app/build/outputs/apk/debug/app-debug.apk
stat -c '%Y %n' $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_gnc_mpc/RocketGncMpc.cpp
```

### الشبكة
- PC: 10.42.0.1 (enp118s0)
- Phone: 10.42.0.174 (USB Ethernet)
- Bridge TCP: tcpin:0.0.0.0:4560
- QGC: adb forward tcp:5760 tcp:5760

### مسار النتائج (نسبي من PROJECT_ROOT)
```
6DOF_v4_pure/results/
├── Qabthah1_*_log.csv          # Python-only
├── sitl/run_*/sitl_log_*.csv   # SITL
└── hitl/run_*/hitl_log_*.csv   # HITL
```
**أعمدة CSV مختلفة**: Python يستخدم `velocity_x_m_s`, `position_x_m`, `altitude_m` (MSL). SITL/HITL يستخدمون `vel_north_m_s`, `vel_down_m_s`, `altitude_m` (relative), `ground_range_m`.

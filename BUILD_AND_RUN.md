---
noteId: "d7a5b5003f5411f1b05cfd0f62ff5ac8"
tags: []

---

# BUILD_AND_RUN — M130 GNC MPC

> **آخر تحديث**: 2026-04-23  
> **الحالة**: شامل ومصحح — جميع المسارات والبريدج صحيحة

---

## 📋 جدول المحتويات

1. [إعداد المسارات](#إعداد-المسارات)
2. [المرحلة 1: Python-only](#المرحلة-1-python-only)
3. [المرحلة 2: SITL](#المرحلة-2-sitl)
4. [المرحلة 2.5: PIL](#المرحلة-25-pil-جديد)
5. [المرحلة 3: HITL](#المرحلة-3-hitl)
6. [بناء APK (Android)](#بناء-apk)
7. [الاتصالات والشبكة](#الاتصالات-والشبكة)
8. [التشخيص السريع](#التشخيص-السريع)
9. [جدول مرجعي](#جدول-مرجعي-سريع)

---

## إعداد المسارات

### تحديد PROJECT_ROOT

```bash
# الطريقة 1: يدوي
export PROJECT_ROOT=/home/hk/workspaces/System/real_sim/mpc/m13

# الطريقة 2: تلقائي
export PROJECT_ROOT=$(find . -maxdepth 2 -name 'm13' -type d | head -1)
if [ ! -d "$PROJECT_ROOT/6DOF_v4_pure" ]; then
  echo "خطأ: m13 غير موجود!"
  exit 1
fi
```

### إعداد البيئة

```bash
export ACADOS_PATH=$(cd $PROJECT_ROOT && find . -maxdepth 2 -name 'acados-main' -type d | head -1)
export LD_LIBRARY_PATH=$ACADOS_PATH/lib:$LD_LIBRARY_PATH
```

---

## المرحلة 1: Python-only

```bash
cd $PROJECT_ROOT/6DOF_v4_pure
pip install -r requirements.txt
python3 rocket_6dof_sim.py
```

**النتيجة**: ملف CSV في `results/Qabthah1_*.csv`

---

## المرحلة 2: SITL

### بناء PX4 (مرة واحدة)

```bash
cd $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot
export LD_LIBRARY_PATH=$ACADOS_PATH/lib:$LD_LIBRARY_PATH
rm -rf build/px4_sitl_default
make px4_sitl_default
```

### نافذة #1: تشغيل PX4

```bash
cd $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot
rm -f build/px4_sitl_default/rootfs/parameters.bson
export LD_LIBRARY_PATH=$ACADOS_PATH/lib:$LD_LIBRARY_PATH
PX4_SYS_AUTOSTART=23000 ./build/px4_sitl_default/bin/px4
```

### نافذة #2: تشغيل Bridge

```bash
cd $PROJECT_ROOT/6DOF_v4_pure
python3 sitl/mavlink_bridge.py
```

**النتيجة**: ملف CSV في `results/sitl/run_*/sitl_log_*.csv`

---

## المرحلة 2.5: PIL (جديد)

### نافذة #1: على ARM device

```bash
cd /home/pi/mpc_dev/6DOF_v4_pure
python3 pil/pil_runner.py --config pil_config.yaml
```

### نافذة #2: Bridge من الـ PC

```bash
cd $PROJECT_ROOT/6DOF_v4_pure
python3 pil/mavlink_bridge_pil.py --pil --pil-ip 192.168.1.100
```

---

## المرحلة 3: HITL

### خطوة 1: بناء APK

```bash
cd $PROJECT_ROOT/AndroidApp
./gradlew clean assembleDebug
```

### خطوة 2: فحص timestamp

```bash
# يجب أن يكون APK أحدث من كود C++
stat -c '%y %n' $PROJECT_ROOT/AndroidApp/app/build/outputs/apk/debug/app-debug.apk
stat -c '%y %n' $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_mpc/RocketMPC.cpp
```

### خطوة 3: إعداد الاتصال

```bash
adb disconnect
adb tcpip 5555
adb connect 192.168.2.138:5555
adb forward tcp:5760 tcp:5760
adb forward tcp:14550 tcp:14550
```

### خطوة 4: تثبيت APK

```bash
adb install -r $PROJECT_ROOT/AndroidApp/app/build/outputs/apk/debug/app-debug.apk
```

### نافذة #1: تشغيل الخدمة على الهاتف

```bash
adb shell am force-stop com.ardophone.px4v17
adb shell am start-foreground-service \
  -n com.ardophone.px4v17/.service.FlightService \
  -a com.ardophone.px4v17.START_FLIGHT \
  --ez hitl true
```

### نافذة #2: Bridge

```bash
cd $PROJECT_ROOT/6DOF_v4_pure
python3 hil/mavlink_bridge_hil.py --hitl
```

**النتيجة**: ملف CSV في `results/hitl/run_*/hitl_log_*.csv`

---

## بناء APK

```bash
cd $PROJECT_ROOT/AndroidApp

# بناء سريع
./gradlew assembleDebug

# بناء كامل مع اختبارات
./gradlew build

# تنظيف كامل (عند الأخطاء الغريبة)
./gradlew clean && ./gradlew --stop && ./gradlew assembleDebug
```

---

## الاتصالات والشبكة

### USB Ethernet

```bash
# على الهاتف: USB Tethering
# على PC:
ip link show
sudo ip addr add 192.168.2.1/24 dev enp118s0  # الواجهة الصحيحة
ping 192.168.2.174
```

### QGC

```bash
sudo apt install qgroundcontrol
qgroundcontrol &
# في QGC: TCP → localhost:5760
```

---

## التشخيص السريع

### فحص المسارات

```bash
ls $PROJECT_ROOT/6DOF_v4_pure/c_generated_code/acados_*.h | wc -l  # يجب >= 20
ls $PROJECT_ROOT/6DOF_v4_pure/mpc/m130_mpc_autopilot.py
ls $PROJECT_ROOT/AndroidApp/app/build/outputs/apk/debug/app-debug.apk
```

### فحص الملفات المولدة

```bash
diff $PROJECT_ROOT/6DOF_v4_pure/c_generated_code/acados_solver_m130_rocket.h \
     $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot/src/modules/rocket_mpc/acados_generated/acados_solver_m130_rocket.h
```

### معالجة الأخطاء

```bash
pkill -f "px4_sitl"
pkill -f "mavlink_bridge"
adb shell am force-stop com.ardophone.px4v17

rm -f $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot/build/px4_sitl_default/rootfs/parameters.bson
adb shell pm clear com.ardophone.px4v17

cd $PROJECT_ROOT/AndroidApp && ./gradlew clean assembleDebug
```

---

## جدول مرجعي سريع

### الملفات الحرجة

| الملف | المسار | النوع |
|-------|--------|-------|
| Python MPC | `6DOF_v4_pure/mpc/m130_mpc_autopilot.py` | مرجع ★ |
| C++ MPC | `AndroidApp/.../rocket_mpc/mpc_controller.cpp` | يجب = Python |
| Bridge SITL | `6DOF_v4_pure/sitl/mavlink_bridge.py` | SITL فقط |
| Bridge HITL | `6DOF_v4_pure/hil/mavlink_bridge_hil.py` | HITL فقط |
| Bridge PIL | `6DOF_v4_pure/pil/mavlink_bridge_pil.py` | PIL فقط |

### أوامر البناء

```bash
# Python: لا بناء
cd $PROJECT_ROOT/6DOF_v4_pure && python3 rocket_6dof_sim.py

# SITL
cd $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot && make px4_sitl_default

# HITL
cd $PROJECT_ROOT/AndroidApp && ./gradlew clean assembleDebug

# PIL (ARM64)
cd $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot && make px4_arm64_v8
```

### أوامر التشغيل

```bash
# المرحلة 1
cd $PROJECT_ROOT/6DOF_v4_pure && python3 rocket_6dof_sim.py

# المرحلة 2 (T1 + T2)
cd $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot && \
  PX4_SYS_AUTOSTART=23000 ./build/px4_sitl_default/bin/px4  # T1
cd $PROJECT_ROOT/6DOF_v4_pure && python3 sitl/mavlink_bridge.py  # T2

# المرحلة 3 (T1 + T2)
adb shell am start-foreground-service ... --ez hitl true  # T1
cd $PROJECT_ROOT/6DOF_v4_pure && python3 hil/mavlink_bridge_hil.py --hitl  # T2

# المرحلة 2.5 (T1 + T2)
python3 pil/pil_runner.py --config pil_config.yaml  # T1 (ARM)
cd $PROJECT_ROOT/6DOF_v4_pure && python3 pil/mavlink_bridge_pil.py --pil --pil-ip 192.168.1.100  # T2 (PC)
```

---

## نصائح إضافية

### قبل البدء

```bash
# تحقق من المتطلبات
export PROJECT_ROOT=/path/to/mpc/m13
cd $PROJECT_ROOT

python3 -c "import acados; print('✓ Acados')" 2>/dev/null || echo "✗ Acados not found"
which adb > /dev/null && echo "✓ ADB" || echo "✗ ADB not found"
ls -d AndroidApp && echo "✓ AndroidApp" || echo "✗ Not found"
[ -d acados-main ] && echo "✓ acados-main" || echo "✗ Not found"
```

### معالجة Deadlock/Crash

```bash
# 1. توقف كل شيء
pkill -f "px4_sitl"
pkill -f "mavlink_bridge"
adb shell am force-stop com.ardophone.px4v17

# 2. نظف
cd $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot
rm -f build/px4_sitl_default/rootfs/parameters.bson
adb shell pm clear com.ardophone.px4v17

# 3. أعد
cd $PROJECT_ROOT/AndroidApp && ./gradlew clean assembleDebug
```

### tmux للنوافذ المتعددة

```bash
tmux new-session -d -s mpc -x 180 -y 50
tmux send-keys -t mpc:0 "cd $PROJECT_ROOT/AndroidApp/app/src/main/cpp/PX4-Autopilot && \
  PX4_SYS_AUTOSTART=23000 ./build/px4_sitl_default/bin/px4" C-m

tmux new-window -t mpc -n bridge
tmux send-keys -t mpc:1 "cd $PROJECT_ROOT/6DOF_v4_pure && python3 sitl/mavlink_bridge.py" C-m

tmux list-windows -t mpc
```

---

**آخر تحديث**: 2026-04-23 ✓ | **حالة**: ✅ كامل ومصحح

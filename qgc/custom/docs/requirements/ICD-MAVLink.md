# ICD — MAVLink Interface for M130 GCS

- **Document ID**: ICD-M130-MAVLINK-001
- **Revision**: A (Foundation)
- **Scope**: واجهة MAVLink 2 بين M130 GCS ووحدة `rocket_mpc` في PX4 على الطائرة
- **Protocol Version** (in `M130_HEARTBEAT_EXTENDED`): `0x00010000` (Major.Minor = 1.0)

---

## 1. القنوات المدعومة

| القناة | الوسيلة | المعدل الأدنى | الاستخدام |
|---|---|---|---|
| Primary | UDP 14550 | 115 kbaud | الرابط التشغيلي الرئيسي |
| Secondary | USB/Serial 921600 | 115 kbaud | رابط بديل / SITL |
| Telemetry | UDP 14445 | 256 kbaud | Relay عالي الإنتاجية |

## 2. Dialect الرسمي

الملف المعياري هو `qgc/custom/mavlink/m130.xml`. كل رسالة تحمل `message_id` في المدى 42000–42255 المخصص لـ M130.

### 2.1 الرسائل الواردة (من الطائرة إلى GCS)

| ID | Name | Rate | Description |
|---|---|---|---|
| 42000 | M130_HEARTBEAT_EXTENDED | 1 Hz | Heartbeat ممتد مع protocol_version و system_state |
| 42001 | M130_GNC_STATE | 20 Hz | حالة GNC الأساسية (stage, attitude, position, velocity) |
| 42002 | M130_MHE_DIAGNOSTICS | 10 Hz | تشخيصات MHE (quality, solve time, cross-val errors) |
| 42003 | M130_MPC_DIAGNOSTICS | 10 Hz | تشخيصات MPC (solve time, SQP iters, status, commands) |
| 42004 | M130_FIN_STATE | 20 Hz | حالة الزعانف الأربع + السيرفو |
| 42005 | M130_EVENT_COUNTERS | 1 Hz | عدادات الأخطاء والأحداث |
| 42006 | M130_SENSOR_HEALTH | 1 Hz | صحة IMU/GPS/Baro/Mag + jamming |
| 42007 | M130_COMMAND_ACK_M130 | event | ACK مخصص لأوامر M130 |

### 2.2 الرسائل الصادرة (من GCS إلى الطائرة)

| ID | Name | Auth | Description |
|---|---|---|---|
| 42100 | M130_COMMAND_ARM | Operator+ | تسليح النظام |
| 42101 | M130_COMMAND_DISARM | Operator+ | نزع التسليح |
| 42102 | M130_COMMAND_HOLD | Safety+ | تجميد المهمة في الحالة الحالية |
| 42103 | M130_COMMAND_ABORT | Safety+ | إنهاء طبيعي للمهمة |
| 42104 | M130_COMMAND_FTS | RSO+DualAuth | إنهاء الطيران القسري |
| 42105 | M130_COMMAND_TUNE | FlightDirector+ | ضبط معلمات MPC/MHE (ضمن حدود آمنة) |
| 42106 | M130_COMMAND_MODE_SWITCH | Operator+ | تبديل وضع الطيران |
| 42107 | M130_COMMAND_CHECKLIST_SIGN | Operator+ | توقيع بند من checklist ما قبل الإطلاق |

## 3. تفاصيل الرسائل الحرجة

### 3.1 `M130_HEARTBEAT_EXTENDED` (42000, 1 Hz)

```
uint32_t protocol_version   // Major<<16 | Minor<<8 | Patch
uint32_t system_state       // bitmask: armed|flight_phase|safety_ok|...
uint32_t uptime_s
uint32_t flight_id          // يُولَّد جديد عند كل Arm
uint16_t flags              // bit 0: mission_critical, bit 1: fts_armed, ...
uint8_t  source_system
uint8_t  source_component
```

**قواعد التحقق**:
- إذا `protocol_version.Major` لا يطابق → رفض كل الرسائل من هذا النظام
- إذا `protocol_version.Minor` < الأدنى المتوقع → تحذير
- فقدان > 1 s → Caution، > 3 s → Warning، > 10 s في BOOST/CRUISE → Emergency

### 3.2 `M130_GNC_STATE` (42001, 20 Hz)

مُعادل عصري لـ `DEBUG_FLOAT_ARRAY` array_id=2 مع تنظيم رسمي:
```
uint64_t time_usec         // Boot time
uint8_t  stage             // 0..3 (see FlightPhase enum)
uint8_t  launched          // 0/1
uint16_t mode              // Current flight mode
float    q_dyn             // [Pa]
float    rho               // [kg/m^3]
float    altitude          // [m] above takeoff
float    airspeed          // [m/s]
float    phi               // roll [deg]
float    theta             // pitch [deg]
float    psi               // yaw [deg]
float    alpha_est         // [deg]
float    gamma_rad         // [rad]
float    pos_downrange     // [m]
float    pos_crossrange    // [m]
float    vel_downrange     // [m/s]
float    vel_down          // [m/s]
float    vel_crossrange    // [m/s]
float    bearing_deg       // [deg]
float    target_range_rem  // [m]
float    iip_lat           // [deg*1e-7 int32 في النسخة المحسّنة]
float    iip_lon
float    iip_alt
```

### 3.3 `M130_MPC_DIAGNOSTICS` (42003, 10 Hz)

```
uint64_t time_usec
uint32_t solve_count
uint32_t fail_count
uint32_t nan_skip_count
uint32_t solve_us             // آخر زمن حل بالـ µs
uint16_t sqp_iter
uint16_t solver_status        // 0 OK, 1 fail, 2 max_iter, ...
float    cost_total
float    cost_stage
float    cost_terminal
float    constraint_violation
float    delta_roll_cmd       // [deg]
float    delta_pitch_cmd      // [deg]
float    delta_yaw_cmd        // [deg]
float    pitch_accel_cmd      // [rad/s^2]
float    yaw_accel_cmd        // [rad/s^2]
```

### 3.4 `M130_COMMAND_FTS` (42104)

الأمر الأخطر. القواعد:
- **يجب** أن يحمل حقل `auth_token_rso` (HMAC-SHA256 من RSO)
- **يجب** أن يحمل `auth_token_safety` (HMAC-SHA256 من Safety Officer)
- **يجب** أن يكون ضمن نافذة زمنية (timestamp في آخر 30 s)
- **يجب** أن يطابق `flight_id` الحالي
- بعد الإرسال يُنتظر `M130_COMMAND_ACK_M130` بـ `result=FTS_ARMED` خلال 500 ms وإلا يُعاد إرسالها 3 مرات

```
uint64_t time_usec
uint32_t flight_id
uint8_t  auth_token_rso[32]     // HMAC-SHA256
uint8_t  auth_token_safety[32]  // HMAC-SHA256
uint8_t  command_type           // 0=full FTS, 1=engine cutoff only
uint8_t  reason_code            // see FtsReasonCode enum
```

## 4. قواعد عامة للبروتوكول

1. **MAVLink 2 signing** مُلزم على كل القنوات في بيئة الإنتاج.
2. جميع الرسائل تخضع لـ sequence check و timeout.
3. تُسجَّل كل رسالة واردة وصادرة في FDR قبل المعالجة المحلية (REQ-M130-GCS-PROT-004).
4. الأوامر المُرسلة تنتظر ACK خلال 2 s مع retry واحد (REQ-M130-GCS-CMD-003).
5. يُرفض أي رسالة خارج نطاق القيم المقبولة المحددة في الـ dialect XML (field `min` و `max`).

## 5. تطور البروتوكول

| Rev | Protocol Version | Changes |
|---|---|---|
| A | 1.0.0 | الإصدار الأول — مشتق من DEBUG_FLOAT_ARRAY array_id=2 |

## 6. خريطة الانتقال من DEBUG_FLOAT_ARRAY

خريطة حرفية في `ICD-DebugFloatArray-legacy.md`. المسار:
1. **المرحلة الأولى** (هذا الـ PR): تعريف m130.xml وتوثيقه فقط
2. **المرحلة الثانية**: توليد كود C++/Python من m130.xml، تشغيل dual-emit في firmware (القديم + الجديد)
3. **المرحلة الثالثة**: GCS يستقبل من الجديد، القديم للتحقق
4. **المرحلة الرابعة**: إزالة DEBUG_FLOAT_ARRAY من firmware وGCS

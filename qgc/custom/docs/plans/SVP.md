# SVP — Software Verification Plan

- **Document ID**: SVP-M130-GCS-001
- **Revision**: A (Foundation)
- **Standard**: DO-178C §6

## 1. Verification Methods

| Method | Usage |
|---|---|
| Inspection | وثائق، تصميم، كود يدوي |
| Analysis | MC/DC، static analysis، timing، memory budget |
| Demonstration | واجهة، تفاعلات، حالات end-to-end |
| Test | unit, integration, system, regression |

## 2. Test Levels

### 2.1 Unit Tests (Qt Test / GoogleTest)
- تغطي كل class في `src/safety/`, `src/logging/`, `src/access/`, `src/protocol/`
- تُشغَّل في CI على Linux/macOS/Windows
- Coverage goal ≥ 90% على هذه الأنوية

### 2.2 QML Tests (Qt Quick Test)
- لكل component جديد في `src/views/` و `src/ui/`
- اختبارات بصرية + accessibility + RTL

### 2.3 Integration Tests
- SITL bridge: تشغيل محاكاة كاملة وتحقق من أن FactGroups تُحدَّث
- Dialect conformance: GCS يفك رسائل محاكاة من m130.xml
- Round-trip: command → ACK → state update → audit entry

### 2.4 System Tests
- سيناريوهات end-to-end مع SITL
- سيناريوهات fail-safe (link loss, solver failure, sensor bad)
- سيناريوهات FTS (dual auth, timeout, replay)

### 2.5 Performance Tests
- قياس latency (≤ 100 ms) و throughput
- قياس استهلاك RAM (≤ 1 GB)
- bench runs في CI على PR

### 2.6 Security Tests
- اختبار محاولات brute-force على UserManager (≥ 5 محاولات → lockout)
- اختبار توقيع MAVLink (رفض رسائل غير موقَّعة)
- اختبار replay protection
- gitleaks/secret-scanning في CI

### 2.7 Fuzz Tests
- MAVLink parser fuzz (AFL/libFuzzer)
- FDR playback fuzz (ملفات تالفة)

## 3. Coverage Requirements (per REQ-M130-GCS-QA-001/002)

| Component | Statement | Branch | MC/DC |
|---|---|---|---|
| Safety Kernel | 100% | 100% | 100% |
| Access Layer | 100% | 100% | 90% |
| Protocol Layer | 95% | 90% | — |
| Logging Layer | 90% | 85% | — |
| Telemetry Layer | 85% | 80% | — |
| UI (QML) | 70% | — | — |

## 4. Test Environment
- CI: GitHub Actions linux/macos/windows
- Hardware-in-the-loop: آلة اختبار مخصصة (مرحلة لاحقة)
- SITL: تُبنى من مشروع m13/6DOF_v4_pure

## 5. Defect Management
- كل defect يفتح issue على GitHub
- severity: 1 (blocker), 2 (major), 3 (minor), 4 (trivial)
- RCA مطلوب لكل severity 1/2 قبل الإغلاق

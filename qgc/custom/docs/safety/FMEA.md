# FMEA — Failure Mode and Effects Analysis

- **Document ID**: FMEA-M130-GCS-001
- **Revision**: A (Foundation)
- **Standard**: SAE J1739, MIL-STD-1629

## 1. Scoring Scale (1-10 each; RPN = S × O × D)

- **Severity (S)**: 1 = no effect, 10 = catastrophic
- **Occurrence (O)**: 1 = extremely rare, 10 = very frequent
- **Detection (D)**: 1 = detected immediately, 10 = undetectable

Action threshold: RPN ≥ 100 OR S = 10 OR S × O ≥ 50.

## 2. FMEA Table (الأساس — يتوسع بالـ PRs القادمة)

| Component | Failure Mode | Cause | Local Effect | System Effect | S | O | D | RPN | Mitigation | Post-RPN |
|---|---|---|---|---|---|---|---|---|---|---|
| Protocol Parser | Buffer overflow | Malformed MAVLink | Crash | Loss of GCS | 9 | 2 | 3 | 54 | Size check + fuzz testing + ASan | 18 |
| Protocol Parser | Version mismatch accepted | Missing check | Wrong decoding | Garbage data shown | 8 | 3 | 4 | 96 | ProtocolVersion guard + reject unknown | 16 |
| MessageRouter | Silent drop | No error path | Missing update | Stale display | 7 | 4 | 6 | 168 | Per-message counter + Watchdog staleness | 28 |
| Watchdog | Timer drift | OS scheduling | Late alert | Delayed response | 8 | 4 | 5 | 160 | Monotonic clock + self-check + independent thread | 32 |
| MissionStateMachine | Illegal transition accepted | Guard bug | Undefined state | Unsafe behavior | 10 | 2 | 6 | 120 | Guards + property-based tests + MC/DC | 20 |
| AlertManager | Alert lost in flood | Queue overflow | Missed alert | Missed hazard | 9 | 3 | 7 | 189 | Priority queue + dedup + bounded queue + drop-low | 36 |
| AlertManager | Wrong severity | Misclassification | Ignored alert | Missed hazard | 8 | 4 | 6 | 192 | Severity table tests + operator ack + escalation timer | 32 |
| CommandAuthorization | Bypass | Logic flaw | Unauth command sent | Mission risk | 10 | 2 | 5 | 100 | Unit tests + MC/DC + default-deny | 10 |
| FlightTerminationService | False trigger | Logic / user error | FTS sent | Catastrophic | 10 | 2 | 4 | 80 | Dual auth + TOTP + confirm + audit + cooldown | 8 |
| FlightTerminationService | Missed trigger | Comm lost | No FTS | Loss of safety | 10 | 3 | 7 | 210 | Ground retry + autonomous onboard FTS + IIP watch | 60 |
| FlightSafetyEnvelope | Missing limit | Spec gap | Violation not flagged | Safety escape | 9 | 3 | 6 | 162 | Central spec + unit tests + hazard review | 27 |
| FlightDataRecorder | Silent drop | Disk full | Missing record | Loss of evidence | 8 | 3 | 7 | 168 | Free-space monitor + alert + rotation | 32 |
| AuditLogger | Tamper without detection | Missing hash chain | Undetected alter | Loss of forensics | 9 | 2 | 7 | 126 | Hash chain + append-only + offsite replica | 27 |
| ChainOfCustody | Key leak | Storage flaw | Forged sigs | Loss of forensics | 9 | 2 | 5 | 90 | Keychain/HSM + rotation + audit access | 18 |
| UserManager | Brute-force success | Weak hashing | Unauth access | Safety event | 10 | 3 | 6 | 180 | argon2id + lockout + TOTP + alerts on multiple fails | 24 |
| SessionManager | Session fixation | Token reuse | Hijacked session | Unauth commands | 9 | 3 | 6 | 162 | Rotate token on auth + short TTL + reauth on critical | 27 |
| Theme | Colorblind miss warning | Missing cue | Missed warning | Missed hazard | 9 | 4 | 8 | 288 | Shape + sound + 4 palettes + accessibility tests | 36 |
| FlyView QML | Freeze under load | GC pressure | Stale display | Delayed decisions | 7 | 5 | 6 | 210 | QML profiler + lazy loading + bench tests | 42 |
| Translation | RTL layout breaks alerts | i18n defect | Misread text | Wrong decision | 7 | 4 | 5 | 140 | RTL tests + visual diffs | 28 |
| MAVLink Signing | Unsigned message accepted | Config off | Spoofed data | Safety event | 10 | 3 | 5 | 150 | Enforce in config + reject unsigned + alert | 20 |

## 3. Action Plan (RPN ≥ 100 post-mitigation review)
- أي بند بـ Post-RPN ≥ 40 يحتاج review ربع سنوي
- أي بند بـ Post-RPN ≥ 100 غير مقبول — يتطلب mitigation إضافي

## 4. Linkage
- كل mitigation مرتبط بـ REQ في SRS
- اختبار mitigation مسجل في traceability.csv

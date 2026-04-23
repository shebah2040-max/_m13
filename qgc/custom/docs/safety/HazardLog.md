# Hazard Log — M130 GCS

- **Document ID**: HAZ-M130-GCS-001
- **Revision**: A (Foundation)
- **Standard**: MIL-STD-882E; ARP4761 (FHA+PSSA patterns)

## 1. Severity Categories (MIL-STD-882E)

| Cat | Label | Description |
|---|---|---|
| I | Catastrophic | وفاة، فقدان نظام، تلف بيئي كبير |
| II | Critical | إصابة خطيرة، تلف نظام، أضرار بيئية |
| III | Marginal | إصابة طفيفة، تلف محدود |
| IV | Negligible | بلا إصابة، تأثير ضئيل |

## 2. Probability Levels

| Lvl | Label | Frequency |
|---|---|---|
| A | Frequent | كل مهمة تقريباً |
| B | Probable | عدة مرات في الحياة التشغيلية |
| C | Occasional | بعض المرات |
| D | Remote | نادر |
| E | Improbable | شبه مستحيل |

## 3. Hazard Risk Index (HRI Matrix)

|   | A  | B  | C  | D  | E  |
|---|----|----|----|----|----|
| I  | 1  | 1  | 2  | 3  | 4  |
| II | 1  | 2  | 3  | 4  | 5  |
| III| 2  | 3  | 4  | 5  | 6  |
| IV | 3  | 4  | 6  | 6  | 7  |

1-2: Unacceptable | 3-4: Undesirable | 5-6: Acceptable w/ review | 7: Acceptable

## 4. Hazard Register

| ID | Hazard | Cause | Effect | Sev | Prob | HRI | Mitigation | Residual | Status |
|---|---|---|---|---|---|---|---|---|---|
| HAZ-001 | فقدان اتصال MAVLink أثناء BOOST | Link cut / jamming | فقدان معرفة الحالة، تأخر ردود الأفعال | II | C | 3 | Watchdog متعدد الطبقات + IIP استمراري + alerting 4 مستويات | 5 | Mitigated |
| HAZ-002 | عرض قيمة Fact خاطئة | Dialect mismatch أو corruption | قرار مشغل خاطئ | II | D | 4 | Protocol version check + CRC + bounds check + staleness | 6 | Mitigated |
| HAZ-003 | إرسال FTS بالخطأ | مشغل غير مخوَّل أو خطأ بشري | إنهاء قسري غير مطلوب | I | D | 3 | Dual authorization + confirm dialog + TOTP + audit + Safety Officer override | 5 | Mitigated |
| HAZ-004 | فشل FTS عند الحاجة | Link loss + auth timeout | استمرار طيران خطر | I | D | 3 | FTS على الطائرة بذاكرة + timeout آمن + redundant signing + ground-side retry | 4 | Partial |
| HAZ-005 | اختراق نظام المصادقة | Brute-force / credential theft | Unauthorized commands | I | D | 3 | argon2id + lockout + TOTP + audit + TLS 1.3 + key rotation | 5 | Mitigated |
| HAZ-006 | تلاعب بسجل FDR | Tampering for liability | Inability to reconstruct | III | D | 5 | Hash chain + HMAC + offsite replica | 6 | Mitigated |
| HAZ-007 | إرهاق مشغل (cognitive overload) | Alarm flood | Missed critical alert | II | B | 2 | Alert prioritization + master caution/warning + sound differentiation + filter by role | 4 | Partial |
| HAZ-008 | فقدان البيانات بسبب disk full | تشغيل طويل | فقدان سجل | III | C | 4 | Monitoring + rotation + warn at 80% | 5 | Mitigated |
| HAZ-009 | بصر ألوان (colorblind) يفوت warning | عمى ألوان | Safety event missed | III | B | 3 | Colorblind palettes + shape cues + sound | 5 | Mitigated |
| HAZ-010 | جريان خطأ في State Machine | Race condition / bug | Undefined system state | II | D | 4 | Formal state machine + guards + property-based tests + MC/DC | 6 | Mitigated |
| HAZ-011 | فشل solver (MPC/MHE) في الطيران | Numerical issue | فقدان التحكم الفعلي | I | D | 3 | Fallback controller + graceful degradation + alert Warning/Emergency | 4 | Partial |
| HAZ-012 | خرق safety envelope بدون تنبيه | Missing check | Injury / damage | I | E | 4 | Central envelope service + tested بوضوح + alerts | 5 | Mitigated |
| HAZ-013 | Replay attack على MAVLink | Attacker records/replays | Spoofed state | II | D | 4 | MAVLink 2 signing + nonce + timestamp + sequence | 6 | Mitigated |
| HAZ-014 | انقطاع كهرباء المحطة | Power fail | Operations halt | II | C | 3 | UPS (ops) + auto-resume + FDR sync قبل الانهيار | 5 | Ops Mitigation |
| HAZ-015 | خطأ في مصفوفة صلاحيات | Misconfig | Unauthorized action | II | D | 4 | RBAC tests + CAPA on every NC | 6 | Mitigated |
| HAZ-016 | نقص تدريب المشغل | Insufficient training | Wrong decision | II | C | 3 | Mandatory training + annual recurrent + procedure checklists | 5 | Ops Mitigation |
| HAZ-017 | عرض stale بدون إشعار | Watchdog gap | Decision on old data | II | D | 4 | Staleness per-fact + visible age + amber at 500 ms | 6 | Mitigated |
| HAZ-018 | انحياز في GPS (jamming/spoofing) | External attack | Navigation error | II | C | 3 | Jamming detection + cross-check مع MHE + alert | 4 | Partial |
| HAZ-019 | خطأ في RTL/translation يسبب confusion | I18n defect | Wrong operator decision | III | D | 5 | TS validation في CI + review لغوي | 6 | Mitigated |
| HAZ-020 | حرمان خدمة (DoS) على المحطة | Network attack | Unable to command | II | D | 4 | Rate limiting + firewall + dedicated network | 6 | Mitigated |

## 5. Mitigation Traceability
- كل mitigation مرتبط بـ REQ في `requirements/SRS-M130GCS.md`
- كل mitigation يُثبَت فعاليته بواحد من: test / analysis / demonstration

## 6. Update Policy
- أي PR يضيف ميزة أمنية/خطيرة يجب تحديث هذا السجل
- تقييم إعادة HRI سنوياً (minimum) وبعد كل حادثة تشغيلية

# AS9100D Delta over ISO 9001:2015

- **Document ID**: COMP-AS9100D-001
- **Revision**: A

AS9100D يضيف على ISO 9001:2015 متطلبات متخصصة للفضاء والطيران:

| § | AS9100D Addition | Evidence | Status |
|---|---|---|---|
| 4.4.2 | Process approach (aerospace-flavored) | `docs/design/SystemArchitecture.md` | ✓ |
| 8.1.1 | Operational risk management | `docs/safety/HazardLog.md` + `FMEA.md` | ✓ |
| 8.1.2 | Configuration management | `docs/plans/SCMP.md` | ✓ |
| 8.1.3 | Product safety | `docs/safety/HazardLog.md` + `SafetyEnvelope.md` | ✓ |
| 8.1.4 | Counterfeit parts prevention | SBOM + signed dependencies | ✓ (software scope) |
| 8.4.3 | Control of external providers | `docs/plans/SCMP.md` §5 | ✓ |
| 8.5.1.1 | Control of production (software build) | CI/CD + signed artifacts | ✓ |
| 8.5.1.3 | Verification after modification | PR template + regression tests | ✓ |
| 8.5.3 | Property of customers/external providers | N/A (stand-alone product) | N/A |
| 8.5.5 | Post-delivery activities | Release + patch process | Partial |
| 8.7 | Nonconforming output handling | `SQAP.md` §6 | ✓ |
| 9.1.3 | Data analysis & evaluation | CI metrics + codecov | ✓ |
| 9.3.2 | Management review inputs incl. risks | Org-level | N/A |

**Notes**:
- المنتج نظام برمجي أرضي؛ بعض متطلبات AS9100D المادية (parts, counterfeit) لا تنطبق مباشرة
- متطلبات الأمان (product safety) مُغطاة بسجل الأخطار و FMEA
- التتبع الكامل مطلوب — مضمون عبر `traceability.csv`

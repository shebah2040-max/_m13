# ISO 9001:2015 — Compliance Checklist

- **Document ID**: COMP-ISO9001-001
- **Revision**: A

| § | Requirement | Evidence | Status |
|---|---|---|---|
| 4.1 | Organizational context | `docs/design/SystemArchitecture.md` §1 | Partial |
| 4.2 | Interested parties | `docs/plans/SDP.md` §2 | Partial |
| 4.3 | QMS scope | `docs/plans/SQAP.md` §1 | ✓ |
| 4.4 | QMS processes | `docs/plans/SDP.md`, `SCMP.md`, `SVP.md`, `SQAP.md` | ✓ |
| 5.1 | Leadership commitment | Org-level doc (out of scope here) | N/A |
| 5.2 | Policy | Org-level doc | N/A |
| 5.3 | Roles/responsibilities | `docs/plans/SDP.md` §2 | ✓ |
| 6.1 | Risks & opportunities | `docs/safety/HazardLog.md` + `FMEA.md` | ✓ |
| 6.2 | Quality objectives | `docs/plans/SQAP.md` §1 | ✓ |
| 6.3 | Planning of changes | `docs/plans/SCMP.md` §7 | ✓ |
| 7.1 | Resources | `docs/plans/SDP.md` §2, §4 | Partial |
| 7.1.5 | Monitoring/measuring resources | `docs/plans/SVP.md` §4 | Partial |
| 7.2 | Competence | `docs/plans/SQAP.md` §5 | Partial |
| 7.3 | Awareness | Training records (out of scope here) | N/A |
| 7.4 | Communication | PR process + audit log | ✓ |
| 7.5 | Documented information | `docs/` structure + VCS | ✓ |
| 8.1 | Operational planning | `docs/plans/SDP.md` §1 | ✓ |
| 8.2 | Customer requirements | `docs/requirements/SRS-M130GCS.md` | ✓ |
| 8.3 | Design & development | `docs/design/`, `plans/SDP.md` | Partial |
| 8.4 | External providers | SBOM + dependency review | ✓ |
| 8.5 | Production/service | Release process `SCMP.md` §6 | ✓ |
| 8.6 | Release | `SCMP.md` §6 | ✓ |
| 8.7 | Nonconforming outputs | `docs/plans/SQAP.md` §6 | ✓ |
| 9.1 | Performance monitoring | `docs/plans/SQAP.md` §3 | ✓ |
| 9.2 | Internal audit | `docs/plans/SQAP.md` §4 | Partial |
| 9.3 | Management review | Org-level | N/A |
| 10 | Improvement + CAPA | `docs/plans/SQAP.md` §4 + CAPA-Log.md | Partial |

**Foundation PR coverage**: Structural elements ✓. Operational elements (training records, management review, external audits) are organizational and out of scope for repo-level compliance.

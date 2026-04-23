# SDP — Software Development Plan

- **Document ID**: SDP-M130-GCS-001
- **Revision**: A (Foundation)
- **Standard**: ISO/IEC/IEEE 12207:2017, DO-178C §4

## 1. Lifecycle Model
Incremental with dedicated PRs per pillar:
1. **Foundation** (هذا الـ PR): البنية + الـ skeletons + التوثيق
2. **Protocol Maturity**: توليد m130.xml كامل + dual-emit في firmware
3. **Safety Kernel v1**: تنفيذ كامل للـ envelope + FTS + IIP
4. **Views Completion**: كونسولات Operations + PreLaunch + RSO
5. **Access Hardening**: مصادقة كاملة + TLS + key management
6. **Logging Production**: FDR + replay + PDF reports
7. **Analysis & Tuning**: live plotting + parameter tuning
8. **Integration**: SITL/HIL/HAT + weather + NOTAMs
9. **Certification Readiness**: closure of traceability + test maturity

## 2. Roles & Responsibilities

| Role | Responsibilities |
|---|---|
| Software Architect | Approves design; maintains `docs/design/` |
| System Engineer | Owns `docs/requirements/` + traceability |
| Safety Officer | Owns `docs/safety/`; approves changes to Safety Kernel |
| Quality Manager | Owns `docs/plans/` + `docs/compliance/` |
| Developer | Implements per SRS; writes tests to spec |
| Verifier | Independent review; MC/DC analysis on DAL-C items |
| Configuration Manager | CI/CD; release tagging; SBOM |

## 3. Deliverables per Pillar PR
- Updated SRS if new requirements
- Updated traceability.csv
- Design docs where applicable
- Source + tests
- Changelog entry

## 4. Entry/Exit Criteria per PR
**Entry**: pillar scope approved; branch created from main; linter baseline clean
**Exit**:
- All new REQs have tests
- Coverage ≥ 80% on added code
- clang-format/clang-tidy/clazy/ruff/pyright clean
- At least one independent reviewer approved
- `tools/validate-traceability.py` exits 0

## 5. Tools
C++: Qt 6.5, CMake, Ninja, clang 16+, clang-format, clang-tidy, clazy.
Python (tools/CI): uv, ruff, pyright, pytest.
MAVLink: pymavlink for XML validation + code gen.
Docs: Markdown + MkDocs (optional PDF via weasyprint).
Security: cosign (SBOM signing), gitleaks.

# SQAP — Software Quality Assurance Plan

- **Document ID**: SQAP-M130-GCS-001
- **Revision**: A (Foundation)
- **Standards**: ISO 9001:2015, DO-178C §8, AS9100D

## 1. Quality Objectives
- Zero critical defects in production baselines
- 100% requirement traceability (automated gate)
- ≥ 80% code coverage (≥ 100% on Safety Kernel)
- All DO-178C DAL-C objectives satisfied for release baselines
- Clean pre-commit + CI gates on every PR

## 2. QA Process Gates

### 2.1 Pre-Commit (local + CI)
- clang-format
- clang-tidy
- clazy
- ruff + pyright (Python)
- shellcheck (scripts)
- actionlint / zizmor (GHA)
- qmllint (QML)
- vehicle-null-check (QGC custom)
- check-no-qassert (QGC custom)
- yaml-lint, markdownlint

### 2.2 CI Gates
- Build on Linux/macOS/Windows/Android
- Run unit + integration + QML tests
- Coverage report (codecov + local threshold check)
- `tools/validate-traceability.py` (traceability completeness)
- `tools/validate-dialect.py` (m130.xml schema)
- Dependency review
- SBOM generation + cosign signing
- Security scan (Trivy/gitleaks)

### 2.3 Review Gates
- PR requires ≥ 1 independent reviewer approval
- Safety Kernel changes: require Safety Officer approval
- Protocol changes: require System Engineer approval
- Cryptography/Auth: require Security reviewer approval

## 3. Metrics
- **PR cycle time** (target ≤ 72h for typical, ≤ 24h for critical)
- **Defect escape rate** (bugs found post-release ÷ total defects)
- **Coverage per PR** (must not regress)
- **Traceability completeness** (100% required)

## 4. Audits
- **Internal audit** كل 3 أشهر على SDLC
- **External audit** للـ ISO 9001 كل 12 شهراً
- **Security audit** كل 6 أشهر + pen-test سنوي
- CAPA log يُحتفظ به في `docs/plans/CAPA-Log.md`

## 5. Training
- مطور جديد: يجب قراءة SRS + SDP + SQAP + CODING_STYLE + AGENTS قبل أول PR
- Safety Officer: تدريب على MIL-STD-882E + DO-178C
- Security: تدريب على NIST 800-53 + OWASP Top 10

## 6. Non-Conformance
كل non-conformance يُفتح له NCR (Non-Conformance Report):
- معرف `NCR-YYYY-###`
- description, root cause, corrective action, verification
- يُغلَق فقط بعد تحقق من الفاعلية

# DO-178C Objectives — DAL-C

- **Document ID**: COMP-DO178C-DALC-001
- **Revision**: A

> **Scope note**: M130 GCS هي محطة أرضية، والـ DO-178C ينطبق على airborne software. لكن نعتمد **أهدافه كمرجع تصميمي** لأن GCS داعمة مباشرة لطيران حرج. الطبعة الأنسب رسمياً هي **DO-278A** (محطات CNS/ATM) ولها وثيقتها الخاصة.

## Planning Objectives (A-1)

| ID | Objective | Evidence | Status |
|---|---|---|---|
| 1 | PSAC | `docs/plans/SDP.md` (acts as PSAC) | ✓ |
| 2 | SDP | `docs/plans/SDP.md` | ✓ |
| 3 | SVP | `docs/plans/SVP.md` | ✓ |
| 4 | SCMP | `docs/plans/SCMP.md` | ✓ |
| 5 | SQAP | `docs/plans/SQAP.md` | ✓ |
| 6 | Software coding standards | `CODING_STYLE.md` + `AGENTS.md` | ✓ |
| 7 | Software development standards | `docs/plans/SDP.md` §5 | ✓ |
| 8 | Software verification standards | `docs/plans/SVP.md` §3 | ✓ |

## Development Objectives (A-2)

| ID | Objective | Evidence | Status |
|---|---|---|---|
| 1 | High-level requirements | `docs/requirements/SRS-M130GCS.md` | ✓ |
| 2 | Derived HLR identified | Marked in SRS | Foundation |
| 3 | Software architecture | `docs/design/SystemArchitecture.md` | ✓ |
| 4 | LLR from architecture | Per-module design | Pending |
| 5 | Derived LLR identified | Per-module | Pending |
| 6 | Source code developed | `qgc/custom/src/` | Foundation |
| 7 | Executable Object Code produced | CI build artifacts | Foundation |

## Verification Objectives (A-3 to A-7, summary)

| ID | Objective | Evidence | DAL-C Req |
|---|---|---|---|
| A-3 | HLR → SRS consistent w/ system reqs | Traceability CSV | Test/Analysis |
| A-4 | LLR consistent w/ HLR | Per-module traceability | Test/Analysis |
| A-5 | Architecture consistent w/ HLR | Design doc review | Review |
| A-6.1 | Source code complies w/ LLR | Code review + tests | Test |
| A-6.2 | Source code complies w/ architecture | Review | Review |
| A-6.3 | Source code verifiable | Tests per file | Test |
| A-6.4 | Source code conforms to standards | clang-format + lint gates | Tool |
| A-6.5 | Source code traceable to LLR | Traceability CSV | Tool |
| A-7.1 | Test procedures comply w/ test cases | Test design review | Review |
| A-7.2 | Test results comply w/ test procedures | CI results archived | CI |
| A-7.3 | Coverage of HLR | Traceability CSV gate | Tool |
| A-7.4 | Coverage of LLR | Per-module test | Test |
| A-7.5 | Test coverage of software structure (DAL-C: statement + decision) | codecov + branch cov | Tool |

## Configuration Management (A-8)

| ID | Objective | Evidence |
|---|---|---|
| 1 | Config items identified | `docs/plans/SCMP.md` §4 |
| 2 | Baseline established | Git tags + release process |
| 3 | Problem reporting | GitHub Issues + NCR log |
| 4 | Change control | PR process + Safety Officer gate |
| 5 | Archives | Git + artifact registry |
| 6 | Load control | Signed tags |

## Quality Assurance (A-9)

| ID | Objective | Evidence |
|---|---|---|
| 1 | QA activities | `docs/plans/SQAP.md` |
| 2 | Plans + standards reviewed | PR reviews on plan docs |
| 3 | Transition criteria satisfied | PR merge gates |
| 4 | Software conformity | Release checklist |

## Certification Liaison (A-10)

خارج نطاق هذا المستوى (CNS/ATM عبر DO-278A)؛ نوثق **ما نستطيع** للاستعداد:
- PSAC-equivalent: `docs/plans/SDP.md`
- SECI (Software Ecosystem Config Index) equivalent: `docs/plans/SCMP.md` §4
- SAS (Software Accomplishment Summary): سيُنشأ لكل release

## DAL-C Coverage Requirements

- **Statement coverage**: 100% on Safety Kernel
- **Decision coverage**: 100% on Safety Kernel
- **MC/DC**: Required ONLY for DAL-A/B — هنا DAL-C فـ decision coverage كافٍ
- **Structural coverage analysis**: كل fork/branch في `src/safety/` يجب أن يُنفَّذ في اختبار

## Gap Analysis (Foundation)

| Gap | Plan |
|---|---|
| LLR غير موجود كوثيقة رسمية | يُنشأ لكل module في Pillar PRs |
| بعض REQ بلا test بعد | `Status=Pending` في traceability؛ سيُغلَق في Pillar PRs |
| Coverage tool لم يُفعّل | CI workflow سيضيفه في هذا الـ PR |
| Independence verifier | يُحدد reviewer منفصل لـ `src/safety/**` في CODEOWNERS |

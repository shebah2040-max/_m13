# SCMP — Software Configuration Management Plan

- **Document ID**: SCMP-M130-GCS-001
- **Revision**: A (Foundation)
- **Standard**: DO-178C §7, ISO/IEC 12207

## 1. Baselines
- **Foundation Baseline** (tagged `fnd-A`): هيكل المشروع + الـ skeletons
- **Maturity Baselines**: كل Pillar PR يُنتج baseline موسوم `pillar-N`
- **Release Baselines**: موسومة `vX.Y.Z` وفق SemVer

## 2. Version Control
- Git monorepo مع branches محمية:
  - `main`: مستقر، protected، PR-only
  - `devin/*`: فروع عمل
  - `release/*`: فروع إصدار
- **Signed commits** مُلزمة (GPG/SSH signing)
- **Signed tags** لكل baseline

## 3. PR Process
1. إنشاء issue أولاً (مع REQ IDs)
2. إنشاء فرع `devin/<timestamp>-<feature>`
3. كل commit: `<scope>: <title>` + `Refs: REQ-...`
4. ملء PR template
5. CI يجب أن يمر (lint + test + coverage + traceability)
6. مراجعة من ≥ 1 مهندس مستقل
7. Squash-merge إلى main

## 4. Artifacts Under Configuration Control
- `qgc/custom/**` (كل شيء في هذا المجلد)
- `qgc/custom/docs/**` (جميع الوثائق)
- `qgc/custom/mavlink/m130.xml` + المولد
- CI workflows المتعلقة بـ custom
- Release notes + changelog

## 5. SBOM & Supply Chain
- توليد SBOM (cyclonedx) عند كل release
- توقيع SBOM بـ cosign
- Dependency review على كل PR (GitHub dependency-review-action)
- gitleaks في CI لمنع تسريب أسرار

## 6. Release Process
1. إنشاء `release/X.Y.Z` من main
2. تحديث `docs/operations/ReleaseNotes-X.Y.Z.md`
3. تشغيل CI كامل + HIL tests
4. توقيع tag + push
5. توليد AppImage + APK + DMG + MSI
6. نشر + أرشفة binaries مع SBOM

## 7. Change Control
- **Minor change** (bugfix): PR واحد مع reviewer
- **Major change** (requirement change): تحديث SRS + تصميم + موافقة Safety Officer
- **Emergency patch**: hotfix branch من release tag مع expedited review

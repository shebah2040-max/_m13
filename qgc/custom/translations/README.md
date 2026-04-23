# Translations

Qt Linguist `.ts` files for the M130 GCS. Use `lupdate` to refresh:

```bash
cd /home/ubuntu/_m13/qgc
lupdate custom/src -ts custom/translations/m130_ar.ts custom/translations/m130_en.ts
```

Compile with `lrelease` at build time (wired in Pillar 8).

Foundation ships empty `.ts` skeletons so the pipeline is ready for the UI
buildout. Arabic strings are primary (RTL) and English is secondary (LTR).

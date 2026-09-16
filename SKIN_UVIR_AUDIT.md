# Keystone v1.4 — Skin / UV-IR Audit

## Skin Source
- Source: user-supplied OFX binary.
- Skin Tones control surface recovered: parameter labels, ranges, defaults, and six preset names.
- HSL hue/lightness selector geometry is recovered and used. The host-side numeric recipes loaded by the six Skin presets have not yet been fully recovered from the binary. Keystone v1.4 therefore uses conservative recipes matching the recovered preset intent; these recipes are explicitly not represented as bit-identical preset values.

## UV/IR Source
- SpektraFilm is wavelength-resolved; Keystone is not.
- The OFX uses the erf cutoff shape/default edges projected onto a 610/550/450 nm RGB basis, with that limitation documented in source and UI.
